#include "SimulatorManager.h"

#include "SingletonHolder.h"
#include "IPhysicsManager.h"
#include "IThreadManager.h"
#include "IProfilerManager.h"
#include "IGraphicsManager.h"
#include "IConfigManager.h"
#include "IInputManager.h"
#include "ISpacePartitionerManager.h"
#include "ITimeManager.h"
#include "IRaycastManager.h"
#include "ISerializerManager.h"

#include "TimeWaitResult.h"

#include "ParticleSystem.h"
#include "FluidParticleSystem.h"
#include "RigidBodyParticleSystem.h"

#include "GeneralSimulationConfig.h"
#include "GeneralDebugConfig.h"
#include "SceneSimulationConfig.h"
#include "SceneFluidConfig.h"
#include "SceneRecordConfig.h"

#include "KernelMode.h"
#include "RecordMode.h"
#include "ReplaySolver.h"

#include "PartitionSelection.h"

#include "ParticleCountInfo.h"

#include "SemiImplicitEulerSolver.h"
#include "Kernel.h"

#include "SPHBaseSolver.h"

#include "SpecialKey.h"

#include "RunnerHelper.h"
#include "Vector3Utils.h"
#include "BoundingBox.h"

#include "BlowerDef.h"
#include "BlowerType.h"
#include "SceneBlowerConfig.h"
#include "BlowerTimeHandler.h"
#include "BlowerEffectArea.h"
#include "Blower.h"

#include "ThreadingSafety.h"
#include "ThreadEnumeration.h"

#include "RaycastQueryRequest.h"
#include "RaycastHitResult.h"

#include "SerializeParticleSystemLayout.h"
#include "SerializeConstraintLayout.h"
#include "SerializeRecordHeader.h"

#include "SerializeRecordParticleSystemData.h"
#include "SerializeRecordContraintsData.h"
#include "SerializeRecordPendingData.h"

#include "StateSavingOrders.h"
#include "StateSaverHelper.h"
#include "SystemSimulationStateObject.h"

#include "ExitCode.h"

#include "RaycastEnablingFlag.h"

#include "UIField.h"
#include "UIFieldContainer.h"

#include "PushedParticleSystemData.h"

#include "SolverCreationParameter.h"
#include "IterationParameter.h"

#include <fstream>
#include <future>

#define STORM_PROGRESS_REMAINING_TIME_NAME "Remaining time"
#define STORM_FRAME_NUMBER_FIELD_NAME "Frame No"


namespace
{
	template<class ParticleSystemType, class MapType, class ... Args>
	auto& addParticleSystemToMap(MapType &map, unsigned int particleSystemId, Args &&... args)
	{
		std::unique_ptr<Storm::ParticleSystem> particleSystemPtr = std::make_unique<ParticleSystemType>(particleSystemId, std::forward<Args>(args)...);
		return static_cast<ParticleSystemType &>(*(map[particleSystemId] = std::move(particleSystemPtr)));
	}

	class ParticleDataParser
	{
	public:
		enum
		{
			k_printIndex = true
		};

		template<class PolicyType>
		static void parseAppending(std::string &inOutResult, const Storm::ParticleNeighborhoodArray &neighborhood)
		{
			const std::size_t neighborhoodCount = neighborhood.size();
			inOutResult.reserve(inOutResult.size() + 12 + neighborhoodCount * 70);

			inOutResult += "Count :";
			inOutResult += std::to_string(neighborhoodCount);
			
			for (const Storm::NeighborParticleInfo &neighbor : neighborhood)
			{
				inOutResult += "; iter=";
				inOutResult += Storm::toStdString<PolicyType>(neighbor._particleIndex);
				inOutResult += "; r=";
				inOutResult += Storm::toStdString<PolicyType>(neighbor._xijNorm);
				inOutResult += "; xij=";
				inOutResult += Storm::toStdString<PolicyType>(neighbor._xij);
				inOutResult += "; fluid=";
				inOutResult += Storm::toStdString<PolicyType>(neighbor._isFluidParticle);
			}
		}
	};

	template<bool separator, class StreamType, class ContainerType>
	void printToStream(StreamType &stream, const ContainerType &dataContainer, const std::string_view &dataName)
	{
		if constexpr (separator)
		{
			stream <<
				"\n\n\n"
				"********************************************\n"
				"********************************************\n"
				"\n\n";
		}

		stream << dataName << " :\n\n" << Storm::toStdString<ParticleDataParser>(dataContainer) << "\n\n";
	}

	constexpr std::wstring_view k_simulationSpeedBalistName = STORM_TEXT("Simulation Speed");

	class SpeedProfileBalist
	{
	public:
		SpeedProfileBalist(Storm::IProfilerManager* profileMgrPtr) :
			_profileMgrPtr{ profileMgrPtr },
			_active{ true }
		{
			if (profileMgrPtr)
			{
				_profileMgrPtr->startSpeedProfile(k_simulationSpeedBalistName);
			}
		}

		~SpeedProfileBalist()
		{
			if (_active && _profileMgrPtr)
			{
				_profileMgrPtr->endSpeedProfile(k_simulationSpeedBalistName);
			}
		}

		void disable()
		{
			_active = false;
		}

		float getCurrentSpeed() const
		{
			if (_active && _profileMgrPtr)
			{
				return _profileMgrPtr->getCurrentSpeedProfile();
			}

			return 0.f;
		}

	private:
		Storm::IProfilerManager*const _profileMgrPtr;
		bool _active;
	};

	class BlowerCallbacks
	{
	public:
		void notifyStateChanged(const std::size_t blowerId, const Storm::BlowerState newState)
		{
			Storm::IGraphicsManager &graphicMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IGraphicsManager>();
			graphicMgr.changeBlowerState(blowerId, newState);
		}
	};

	template<Storm::BlowerType type, class BlowerEffectArea>
	void appendNewBlower(std::vector<std::unique_ptr<Storm::IBlower>> &inOutBlowerContainer, const Storm::SceneBlowerConfig &blowerConfig)
	{
		std::string_view blowerIntroMsg;
		if constexpr (type != Storm::BlowerType::PulseExplosionSphere)
		{
			if (blowerConfig._fadeInTimeInSeconds > 0.f && blowerConfig._fadeOutTimeInSeconds > 0.f)
			{
				inOutBlowerContainer.emplace_back(std::make_unique<Storm::Blower<type, BlowerEffectArea, Storm::FadeInOutTimeHandler, BlowerCallbacks>>(blowerConfig));
				blowerIntroMsg = "Blower with fadeIn and fadeOut feature created.\n";
			}
			else if (blowerConfig._fadeInTimeInSeconds > 0.f)
			{
				inOutBlowerContainer.emplace_back(std::make_unique<Storm::Blower<type, BlowerEffectArea, Storm::FadeInTimeHandler, BlowerCallbacks>>(blowerConfig));
				blowerIntroMsg = "Blower with fadeIn only feature created.\n";
			}
			else if (blowerConfig._fadeOutTimeInSeconds > 0.f)
			{
				inOutBlowerContainer.emplace_back(std::make_unique<Storm::Blower<type, BlowerEffectArea, Storm::FadeOutTimeHandler, BlowerCallbacks>>(blowerConfig));
				blowerIntroMsg = "Blower with fadeOut only feature created.\n";
			}
			else
			{
				inOutBlowerContainer.emplace_back(std::make_unique<Storm::Blower<type, BlowerEffectArea, Storm::BlowerTimeHandlerBase, BlowerCallbacks>>(blowerConfig));
				blowerIntroMsg = "Blower without fadeIn or fadeOut only feature created.\n";
			}
		}
		else
		{
			inOutBlowerContainer.emplace_back(std::make_unique<Storm::Blower<Storm::BlowerType::PulseExplosionSphere, BlowerEffectArea, Storm::BlowerPulseTimeHandler, BlowerCallbacks>>(blowerConfig));
			blowerIntroMsg = "Explosion Blower Effect created.\n";
		}

		LOG_DEBUG << blowerIntroMsg <<
			"The blower is placed at " << blowerConfig._blowerPosition <<
			", has a dimension of " << blowerConfig._blowerDimension <<
			" and a force of " << blowerConfig._blowerForce <<
			" will start at " << blowerConfig._startTimeInSeconds << "s.";
	}

	inline Storm::Vector3 computeInternalBoundingBoxCorner(const Storm::Vector3 &externalBoundingBoxCorner, const Storm::Vector3 &externalBoundingBoxTranslation, const float coeff)
	{
		// First we need to center the bounding box around { 0, 0, 0 } or it won't scale correctly
		// (the scale will translate/shift the internal bounding box, or the coeff will be applied differently depending on how far we are from 0).
		// Second, we apply back the shift once the scaling was done.
		return (externalBoundingBoxCorner - externalBoundingBoxTranslation) * coeff + externalBoundingBoxTranslation;
	}

	struct RemovedIndexesParam
	{
		bool _shouldConsiderWall;
		std::vector<std::size_t> _outRemovedIndexes;
	};

	template<class ValueType>
	void removeFromIndex(std::vector<ValueType> &container, const std::vector<std::size_t> &indexesToRemove)
	{
		const std::size_t countToRemove = indexesToRemove.size();
		if (countToRemove == 1)
		{
			container.erase(std::begin(container) + indexesToRemove[0]);
			return;
		}

		const std::size_t containerElemCount = container.size();
		const std::size_t lastToRemoveIndex = countToRemove - 1;
		std::size_t startRemoveIndex = 1;
		std::size_t offset = 1;
		std::size_t index = indexesToRemove[0];
		std::size_t posToKeep = index + offset;
		for (; posToKeep < containerElemCount;)
		{
			if (posToKeep != indexesToRemove[startRemoveIndex])
			{
				container[index] = std::move(container[posToKeep]);
				++index;
				posToKeep = index + offset;
			}
			else
			{
				++offset;
				posToKeep = index + offset;

				if (startRemoveIndex < lastToRemoveIndex)
				{
					++startRemoveIndex;
				}
				else
				{
					// No more remove, so just flush the remaining elements, and leave the loop.
					for (; posToKeep < containerElemCount; ++index, ++posToKeep)
					{
						container[index] = std::move(container[posToKeep]);
					}
					break;
				}
			}
		}

		while (offset != 0)
		{
			container.pop_back();
			--offset;
		}
	}

	void removeParticleInsideRbPosition(std::vector<Storm::Vector3> &inOutParticlePositions, const Storm::ParticleSystemContainer &allParticleSystem, const float particleRadius, RemovedIndexesParam* outRemovedIndexes)
	{
		const std::size_t particleSystemCount = allParticleSystem.size();

		const auto computeBoxCoordinate = [particleRadiusPlusMargin = particleRadius + 0.0000001f](std::pair<Storm::Vector3, Storm::Vector3> &box, const std::vector<Storm::Vector3> &currentPSystemPositions, const auto &getterCoordFunc)
		{
			const auto minMaxElems = std::minmax_element(std::execution::par, std::begin(currentPSystemPositions), std::end(currentPSystemPositions), [&getterCoordFunc](const Storm::Vector3 &vect1, const Storm::Vector3 &vect2)
			{
				return getterCoordFunc(vect1) < getterCoordFunc(vect2);
			});

			getterCoordFunc(box.first) = getterCoordFunc(*minMaxElems.first) - particleRadiusPlusMargin;
			getterCoordFunc(box.second) = getterCoordFunc(*minMaxElems.second) + particleRadiusPlusMargin;
		};

		if (outRemovedIndexes != nullptr)
		{
			outRemovedIndexes->_outRemovedIndexes.reserve(inOutParticlePositions.size());
		}

		auto thresholdToEliminate = std::end(inOutParticlePositions);

		for (const auto &particleSystemPair : allParticleSystem)
		{
			const Storm::ParticleSystem &currentPSystem = *particleSystemPair.second;
			if (!currentPSystem.isFluids())
			{
				const std::vector<Storm::Vector3> &currentPSystemPositions = currentPSystem.getPositions();

				LOG_DEBUG << "Processing solid particle system " << currentPSystem.getId() << " with " << currentPSystemPositions.size() << " particles";

				std::pair<Storm::Vector3, Storm::Vector3> externalBoundingBox{ Storm::initVector3ForMin(), Storm::initVector3ForMax() };
				computeBoxCoordinate(externalBoundingBox, currentPSystemPositions, [](auto &vect) -> auto& { return vect.x(); });
				computeBoxCoordinate(externalBoundingBox, currentPSystemPositions, [](auto &vect) -> auto& { return vect.y(); });
				computeBoxCoordinate(externalBoundingBox, currentPSystemPositions, [](auto &vect) -> auto& { return vect.z(); });

				const Storm::Vector3 externalBoundingBoxTranslation = (externalBoundingBox.second + externalBoundingBox.first) / 2.f;

				float coeff = 0.96f;
				std::pair<Storm::Vector3, Storm::Vector3> internalBoundingBox{ 
					computeInternalBoundingBoxCorner(externalBoundingBox.first, externalBoundingBoxTranslation, coeff),
					computeInternalBoundingBoxCorner(externalBoundingBox.second, externalBoundingBoxTranslation, coeff)
				};

				const auto currentPSystemPositionBegin = std::begin(currentPSystemPositions);
				const auto currentPSystemPositionEnd = std::end(currentPSystemPositions);

				bool hasInternalBoundingBox = false;

				// Try to isolate an internal bounding box... Work only if the particle system is hollow (99% of the time).
				// This is an optimization. Try 9 times with a smaller internal bounding box each time.
				for (std::size_t iter = 0; iter < 9; ++iter)
				{
					if (std::find_if(std::execution::par, currentPSystemPositionBegin, currentPSystemPositionEnd, [&internalBoundingBox](const Storm::Vector3 &rbPPosition)
					{
						return Storm::isInsideBoundingBox(internalBoundingBox.first, internalBoundingBox.second, rbPPosition);
					}) == currentPSystemPositionEnd)
					{
						hasInternalBoundingBox = true;
						break;
					}

					coeff -= 0.1f;

					internalBoundingBox.first = computeInternalBoundingBoxCorner(externalBoundingBox.first, externalBoundingBoxTranslation, coeff);
					internalBoundingBox.second = computeInternalBoundingBoxCorner(externalBoundingBox.second, externalBoundingBoxTranslation, coeff);
				}

				if (outRemovedIndexes == nullptr)
				{
					if (hasInternalBoundingBox)
					{
						LOG_DEBUG << "Solid particle system " << currentPSystem.getId() << " is hollow, we will optimize the skin colliding algorithm by using an internal bounding box";

						thresholdToEliminate = std::partition(std::execution::par, std::begin(inOutParticlePositions), thresholdToEliminate, [&currentPSystemPositionBegin, &currentPSystemPositionEnd, &particleRadius, &externalBoundingBox, &internalBoundingBox](const Storm::Vector3 &particlePos)
						{
							if (
								!Storm::isInsideBoundingBox(internalBoundingBox.first, internalBoundingBox.second, particlePos) &&
								Storm::isInsideBoundingBox(externalBoundingBox.first, externalBoundingBox.second, particlePos)
								)
							{
								return std::find_if(std::execution::par, currentPSystemPositionBegin, currentPSystemPositionEnd, [&particlePos, particleRadius](const Storm::Vector3 &rbPPosition)
								{
									return
										std::fabs(rbPPosition.x() - particlePos.x()) < particleRadius &&
										std::fabs(rbPPosition.y() - particlePos.y()) < particleRadius &&
										std::fabs(rbPPosition.z() - particlePos.z()) < particleRadius;
								}) == currentPSystemPositionEnd;
							}

							return true;
						});
					}
					else
					{
						thresholdToEliminate = std::partition(std::execution::par, std::begin(inOutParticlePositions), thresholdToEliminate, [&currentPSystemPositionBegin, &currentPSystemPositionEnd, &particleRadius, &externalBoundingBox](const Storm::Vector3 &particlePos)
						{
							if (Storm::isInsideBoundingBox(externalBoundingBox.first, externalBoundingBox.second, particlePos))
							{
								return std::find_if(std::execution::par, currentPSystemPositionBegin, currentPSystemPositionEnd, [&particlePos, particleRadius](const Storm::Vector3 &rbPPosition)
								{
									return
										std::fabs(rbPPosition.x() - particlePos.x()) < particleRadius &&
										std::fabs(rbPPosition.y() - particlePos.y()) < particleRadius &&
										std::fabs(rbPPosition.z() - particlePos.z()) < particleRadius;
								}) == currentPSystemPositionEnd;
							}

							return true;
						});
					}
				}
				else if (!currentPSystem.isWall() || (outRemovedIndexes->_shouldConsiderWall && currentPSystem.isWall()))
				{
					// Since we need to keep what indexes we removed (because in the case of a state file loading, the other data structures were created and we need to mirror the particle removals to them too).
					// But, to keep indexes consistent, we'll use stable_partition instead of partition, which is far slower + We'll need to lock mutexes.

					std::vector<std::size_t> &removedIndexRef = outRemovedIndexes->_outRemovedIndexes;

					std::mutex _removedIndexMutexes;

					if (hasInternalBoundingBox)
					{
						LOG_DEBUG << "Solid particle system " << currentPSystem.getId() << " is hollow, we will optimize the skin colliding algorithm by using an internal bounding box. We'll save the indexes removed so the algorithm will be slower.";

						thresholdToEliminate = std::stable_partition(std::execution::par, std::begin(inOutParticlePositions), thresholdToEliminate, [&](const Storm::Vector3 &particlePos)
						{
							if (
								!Storm::isInsideBoundingBox(internalBoundingBox.first, internalBoundingBox.second, particlePos) &&
								Storm::isInsideBoundingBox(externalBoundingBox.first, externalBoundingBox.second, particlePos)
								)
							{
								if (std::find_if(std::execution::par, currentPSystemPositionBegin, currentPSystemPositionEnd, [&particlePos, particleRadius](const Storm::Vector3 &rbPPosition)
								{
									return
										std::fabs(rbPPosition.x() - particlePos.x()) < particleRadius &&
										std::fabs(rbPPosition.y() - particlePos.y()) < particleRadius &&
										std::fabs(rbPPosition.z() - particlePos.z()) < particleRadius;
								}) != currentPSystemPositionEnd)
								{
									// Works because it is contiguous in memory
									std::lock_guard<std::mutex> lock{ _removedIndexMutexes };
									removedIndexRef.emplace_back(static_cast<std::size_t>(&particlePos - &*std::begin(inOutParticlePositions)));

									return false;
								}
							}

							return true;
						});
					}
					else
					{
						LOG_DEBUG << "We'll save the particle removed indexes due to collision with rigid body " << currentPSystem.getId() << ". Since we'll keep index, algorithm will be slower.";

						thresholdToEliminate = std::stable_partition(std::execution::par, std::begin(inOutParticlePositions), thresholdToEliminate, [&](const Storm::Vector3 &particlePos)
						{
							if (Storm::isInsideBoundingBox(externalBoundingBox.first, externalBoundingBox.second, particlePos))
							{
								if (std::find_if(std::execution::par, currentPSystemPositionBegin, currentPSystemPositionEnd, [&particlePos, particleRadius](const Storm::Vector3 &rbPPosition)
								{
									return
										std::fabs(rbPPosition.x() - particlePos.x()) < particleRadius &&
										std::fabs(rbPPosition.y() - particlePos.y()) < particleRadius &&
										std::fabs(rbPPosition.z() - particlePos.z()) < particleRadius;
								}) != currentPSystemPositionEnd)
								{
									// Works because it is contiguous in memory
									std::lock_guard<std::mutex> lock{ _removedIndexMutexes };
									removedIndexRef.emplace_back(static_cast<std::size_t>(&particlePos - &*std::begin(inOutParticlePositions)));

									return false;
								}
							}

							return true;
						});
					}

					std::sort(std::execution::par, std::begin(removedIndexRef), std::end(removedIndexRef));
				}
			}
		}

		for (std::size_t toRemove = std::end(inOutParticlePositions) - thresholdToEliminate; toRemove > 0; --toRemove)
		{
			inOutParticlePositions.pop_back();
		}
	}

	float computeCFLDistance(const Storm::SceneSimulationConfig &sceneSimulationConfig)
	{
		const float maxDistanceAllowed = sceneSimulationConfig._particleRadius * 2.f;
		return sceneSimulationConfig._cflCoeff * maxDistanceAllowed;
	}

	constexpr float getMinCLFTime()
	{
		return 0.0000001f;
	}

	template<bool shouldIncrease>
	void changeDeltaTimeStep()
	{
		const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

		assert(singletonHolder.getSingleton<Storm::IConfigManager>().userCanModifyTimestep() && "We shouldn't be able to change the time step in replay mode or when using CFL");

		Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();
		if constexpr (shouldIncrease)
		{
			timeMgr.increaseCurrentPhysicsDeltaTime();
		}
		else // decrease
		{
			timeMgr.decreaseCurrentPhysicsDeltaTime();
		}
	}

	void computeProgression(std::wstring &outResult, const float speed, const float endTime, const float currentTimeInSecond)
	{
		outResult.clear();

		if (speed > 0.f)
		{
			const float remainingTimeToSimulateInSecond = endTime - currentTimeInSecond;
			const float expectedRemainingSimulationTimeInSecond = remainingTimeToSimulateInSecond / speed;

			outResult += std::filesystem::path(Storm::toStdString(std::chrono::seconds{ static_cast<long long>(std::roundf(expectedRemainingSimulationTimeInSecond)) })).wstring();

			outResult += STORM_TEXT(" (");

			const float progressPercent = (currentTimeInSecond / endTime) * 100.f;

			const std::size_t progressPercentStrLength = static_cast<std::size_t>(_scwprintf(L"%.*f", 2, progressPercent));
			const std::size_t currentSize = outResult.size();
			outResult.resize(currentSize + progressPercentStrLength + 1);

			swprintf_s(&outResult[currentSize], progressPercentStrLength + 1, L"%.*f", 2, progressPercent);
			outResult.back() = STORM_TEXT('%');

			outResult += STORM_TEXT(")");
		}
		else
		{
			outResult += STORM_TEXT("N/A");
		}
	}
}

Storm::SimulatorManager::SimulatorManager() :
	_raycastFlag{ Storm::RaycastEnablingFlag::Disabled },
	_runExitCode{ Storm::ExitCode::k_success },
	_reinitFrameAfter{ false },
	_progressRemainingTime{ STORM_TEXT("N/A") },
	_uiFields{ std::make_unique<Storm::UIFieldContainer>() },
	_frameAdvanceCount{ -1 },
	_currentFrameNumber{ 0 }
{
	(*_uiFields)
		.bindField(STORM_FRAME_NUMBER_FIELD_NAME, _currentFrameNumber);
}

Storm::SimulatorManager::~SimulatorManager() = default;

void Storm::SimulatorManager::initialize_Implementation()
{
	LOG_COMMENT << "Initializing the simulator";

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

	/* Check simulation validity */
	const Storm::ParticleCountInfo particleInfo{ _particleSystem };
	if (!configMgr.getGeneralSimulationConfig()._allowNoFluid)
	{
		if (particleInfo._fluidParticleCount == 0)
		{
			Storm::throwException<Storm::Exception>("There is no fluid particles when we initialized the simulation and we didn't allow simulation without fluids");
		}
	}

	LOG_COMMENT <<
		"We'll run a simulation with :\n"
		"- Total particles count involved in the simulation : " << particleInfo._totalParticleCount << "\n"
		"- Fluid particles count : " << particleInfo._fluidParticleCount << "\n"
		"- Total rigid bodies particles count : " << particleInfo._rigidbodiesParticleCount << "\n"
		"- Static rigid bodies particles count : " << particleInfo._staticRigidbodiesParticleCount << "\n"
		"- Dynamic rigid bodies particles count : " << particleInfo._dynamicRigidbodiesParticleCount
		;

	/* initialize the Selector */

	_particleSelector.initialize();

	_kernelHandler.initialize();

	Storm::IThreadManager &threadMgr = singletonHolder.getSingleton<Storm::IThreadManager>();
	threadMgr.setCurrentThreadPriority(configMgr.getUserSetThreadPriority());

	const bool isReplayMode = configMgr.isInReplayMode();

	if (isReplayMode)
	{
		Storm::ISerializerManager &serializerMgr = singletonHolder.getSingleton<Storm::ISerializerManager>();
		Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();

		_frameBefore = std::make_unique<Storm::SerializeRecordPendingData>();
		Storm::SerializeRecordPendingData &frameBefore = *_frameBefore;

		if (!serializerMgr.obtainNextFrame(frameBefore))
		{
			LOG_ERROR << "There is no frame to simulate inside the current record. The application will stop.";
		}

		const Storm::SceneRecordConfig &sceneRecordConfig = configMgr.getSceneRecordConfig();
		this->applyReplayFrame(frameBefore, sceneRecordConfig._recordFps);
	}
	else
	{
		/* Initialize kernels */

		Storm::initializeKernels(this->getKernelLength());
	}

	/* Initialize inputs */

	Storm::IInputManager &inputMgr = singletonHolder.getSingleton<Storm::IInputManager>();
	inputMgr.bindKey(Storm::SpecialKey::KC_E, [this]() { this->tweekBlowerEnabling(); });
	inputMgr.bindKey(Storm::SpecialKey::KC_R, [this]() { this->tweekRaycastEnabling(); });
	inputMgr.bindKey(Storm::SpecialKey::KC_Y, [this]() { this->cycleSelectedParticleDisplayMode(); });
	inputMgr.bindKey(Storm::SpecialKey::KC_I, [this]() { this->resetReplay_SimulationThread(); });
	inputMgr.bindKey(Storm::SpecialKey::KC_C, [this]() { this->executeAllForcesCheck(); });
	inputMgr.bindKey(Storm::SpecialKey::KC_N, [this]() { this->advanceOneFrame(); });
	inputMgr.bindKey(Storm::SpecialKey::KC_X, [this]() { this->saveSimulationState(); });
	inputMgr.bindKey(Storm::SpecialKey::KC_F3, [this]() { this->requestCycleColoredSetting(); });

	if (configMgr.userCanModifyTimestep())
	{
		inputMgr.bindKey(Storm::SpecialKey::KC_2, []() { changeDeltaTimeStep<true>(); });
		inputMgr.bindKey(Storm::SpecialKey::KC_1, []() { changeDeltaTimeStep<false>(); });
		inputMgr.bindMouseWheel([this, &configMgr](int axisRelativeIncrement)
		{
			assert(configMgr.userCanModifyTimestep() && "We shouldn't be able to decrease the time step in replay mode or if we compute the CFL coefficient");

			const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
			Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();

			if (axisRelativeIncrement > 0)
			{
				timeMgr.increasePhysicsDeltaTimeStepSize();
			}
			else if (axisRelativeIncrement < 0)
			{
				timeMgr.decreasePhysicsDeltaTimeStepSize();
			}
		});
	}

	Storm::IRaycastManager &raycastMgr = singletonHolder.getSingleton<Storm::IRaycastManager>();
	inputMgr.bindMouseLeftClick([this, &raycastMgr, &singletonHolder, isReplayMode](int xPos, int yPos, int width, int height)
	{
		if (_raycastFlag != Storm::RaycastEnablingFlag::Disabled)
		{
			if (isReplayMode)
			{
				this->refreshParticlePartition();
			}

			raycastMgr.queryRaycast(Storm::Vector2{ xPos, yPos }, std::move(Storm::RaycastQueryRequest{ [this, &singletonHolder](std::vector<Storm::RaycastHitResult> &&result)
			{
				bool hasMadeSelectionChanges;

				const Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();

				if (result.empty())
				{
					hasMadeSelectionChanges = _particleSelector.clearParticleSelection();
					LOG_DEBUG << "No particle touched";
				}
				else
				{
					const Storm::RaycastHitResult &firstHit = result[0];

					LOG_DEBUG <<
						"Raycast touched particle " << firstHit._particleId << " inside system id " << firstHit._systemId << "\n"
						"Hit Position : " << firstHit._hitPosition;

					hasMadeSelectionChanges = _particleSelector.setParticleSelection(firstHit._systemId, firstHit._particleId);
					if (hasMadeSelectionChanges)
					{
						const Storm::ParticleSystem &selectedPSystem = *_particleSystem[firstHit._systemId];

						_particleSelector.setSelectedParticleSumForce(selectedPSystem.getForces()[firstHit._particleId]);
						_particleSelector.setSelectedParticleVelocity(selectedPSystem.getVelocity()[firstHit._particleId]);
						_particleSelector.setSelectedParticlePressureForce(selectedPSystem.getTemporaryPressureForces()[firstHit._particleId]);
						_particleSelector.setSelectedParticleViscosityForce(selectedPSystem.getTemporaryViscosityForces()[firstHit._particleId]);

						if (!selectedPSystem.isFluids())
						{
							const Storm::RigidBodyParticleSystem &pSystemAsRb = static_cast<const Storm::RigidBodyParticleSystem &>(selectedPSystem);
							_particleSelector.setRbPosition(pSystemAsRb.getRbPosition());
							_particleSelector.setRbTotalForce(pSystemAsRb.getRbTotalForce());
						}
						else
						{
							_particleSelector.clearRbTotalForce();
						}

						_particleSelector.logForceComponents();
					}
				}

				if (hasMadeSelectionChanges && timeMgr.simulationIsPaused())
				{
					this->pushParticlesToGraphicModule(true);
				}
			} }.addPartitionFlag(Storm::PartitionSelection::DynamicRigidBody)
			   .addPartitionFlag(STORM_IS_BIT_ENABLED(_raycastFlag, Storm::RaycastEnablingFlag::Fluids), Storm::PartitionSelection::Fluid)
			   .firstHitOnly())
			);
		}
	});

	inputMgr.bindMouseMiddleClick([this, &raycastMgr, &singletonHolder, isReplayMode](int xPos, int yPos, int width, int height)
	{
		if (_raycastFlag != Storm::RaycastEnablingFlag::Disabled)
		{
			if (isReplayMode)
			{
				this->refreshParticlePartition();
			}

			raycastMgr.queryRaycast(Storm::Vector2{ xPos, yPos }, std::move(Storm::RaycastQueryRequest{ [this, &singletonHolder](std::vector<Storm::RaycastHitResult> &&result)
			{
				if (result.empty())
				{
					LOG_DEBUG << "No particle touched";
				}
				else
				{
					const Storm::RaycastHitResult &firstHit = result[0];

					Storm::IGraphicsManager &graphicMgr = singletonHolder.getSingleton<Storm::IGraphicsManager>();
					graphicMgr.setTargetPositionTo(firstHit._hitPosition);

					LOG_DEBUG << "Camera target set to " << firstHit._hitPosition;
				}
			} }
				.addPartitionFlag(Storm::PartitionSelection::DynamicRigidBody)
				.addPartitionFlag(Storm::PartitionSelection::StaticRigidBody)
				.addPartitionFlag(Storm::PartitionSelection::Fluid)
				.firstHitOnly())
			);
		}
	});

	/* Register this thread as the simulator thread for the speed profiler */
	Storm::IProfilerManager &profilerMgr = singletonHolder.getSingleton<Storm::IProfilerManager>();
	profilerMgr.registerCurrentThreadAsSimulationThread(k_simulationSpeedBalistName);

	const bool autoEndSimulation = configMgr.getSceneSimulationConfig()._endSimulationPhysicsTimeInSeconds != -1.f;
	if (autoEndSimulation)
	{
		(*_uiFields)
			.bindField(STORM_PROGRESS_REMAINING_TIME_NAME, _progressRemainingTime)
			;
	}
}

Storm::ExitCode Storm::SimulatorManager::run()
{
	switch (Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>().getSceneRecordConfig()._recordMode)
	{
	case Storm::RecordMode::None:
	case Storm::RecordMode::Record:
		return this->runSimulation_Internal();

	case Storm::RecordMode::Replay:
		return this->runReplay_Internal();

	default:
		__assume(false);
		Storm::throwException<Storm::Exception>("Unknown record mode!");
	}
}

Storm::ExitCode Storm::SimulatorManager::runReplay_Internal()
{
	LOG_COMMENT << "Starting replay loop";

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
	const Storm::SceneSimulationConfig &sceneSimulationConfig = configMgr.getSceneSimulationConfig();

	Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();
	Storm::IThreadManager &threadMgr = singletonHolder.getSingleton<Storm::IThreadManager>();
	Storm::IProfilerManager* profilerMgrNullablePtr = configMgr.getGeneralDebugConfig()._profileSimulationSpeed ? singletonHolder.getFacet<Storm::IProfilerManager>() : nullptr;

	const Storm::SceneRecordConfig &sceneRecordConfig = configMgr.getSceneRecordConfig();
	Storm::ISerializerManager &serializerMgr = singletonHolder.getSingleton<Storm::ISerializerManager>();

	const bool autoEndSimulation = sceneSimulationConfig._endSimulationPhysicsTimeInSeconds != -1.f;
	bool hasAutoEndSimulation = false;

	Storm::SerializeRecordPendingData &frameBefore = *_frameBefore;
	Storm::SerializeRecordPendingData frameAfter;

	this->refreshParticlePartition(false);

	float expectedReplayFps;
	if (sceneRecordConfig._replayRealTime)
	{
		expectedReplayFps = serializerMgr.getRecordHeader()._recordFrameRate;
		if (timeMgr.getExpectedFrameFPS() != expectedReplayFps)
		{
			frameAfter = frameBefore;
		}
	}
	else
	{
		expectedReplayFps = timeMgr.getExpectedFrameFPS();
	}

	const float physicsFixedDeltaTime = 1.f / expectedReplayFps;
	timeMgr.setCurrentPhysicsDeltaTime(physicsFixedDeltaTime);

	const std::chrono::microseconds fullFrameWaitTime{ static_cast<std::chrono::microseconds::rep>(std::roundf(1000000.f / expectedReplayFps)) };
	auto startFrame = std::chrono::high_resolution_clock::now();

	std::vector<Storm::SerializeRecordContraintsData> recordedConstraintsData;

	do
	{
		SpeedProfileBalist simulationSpeedProfile{ profilerMgrNullablePtr };

		Storm::TimeWaitResult simulationState;
		if (sceneRecordConfig._replayRealTime)
		{
			const auto waitTimeCompensation = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startFrame);
			if (fullFrameWaitTime < waitTimeCompensation)
			{
				simulationState = timeMgr.getStateNoSyncWait();
			}
			else
			{
				simulationState = timeMgr.waitForTime(fullFrameWaitTime - waitTimeCompensation);
			}

			startFrame = std::chrono::high_resolution_clock::now();
		}
		else
		{
			simulationState = sceneSimulationConfig._simulationNoWait ? timeMgr.getStateNoSyncWait() : timeMgr.waitNextFrame();
		}

		switch (simulationState)
		{
		case Storm::TimeWaitResult::Exit:
			if (hasAutoEndSimulation && profilerMgrNullablePtr)
			{
				LOG_COMMENT <<
					"Simulation average speed was " <<
					profilerMgrNullablePtr->getSpeedProfileAccumulatedTime() / static_cast<float>(_currentFrameNumber);
			}

			return _runExitCode;

		case TimeWaitResult::Pause:
			simulationSpeedProfile.disable();

			// Takes time to process messages that came from other threads.
			threadMgr.processCurrentThreadActions();
			
			if (sceneSimulationConfig._simulationNoWait)
			{
				// Eh... this is paused so free a little Cpu will you ;)... No need to spin lock even though we said "run as fast as possible" when we're paused...
				std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
			}
			continue;

		case TimeWaitResult::Continue:
		default:
			break;
		}

		threadMgr.processCurrentThreadActions();

		_kernelHandler.update(timeMgr.getCurrentPhysicsElapsedTime());

		if (_reinitFrameAfter)
		{
			if (timeMgr.getExpectedFrameFPS() != expectedReplayFps)
			{
				frameAfter = frameBefore;
			}

			_reinitFrameAfter = false;
		}

		if (Storm::ReplaySolver::replayCurrentNextFrame(_particleSystem, frameBefore, frameAfter, expectedReplayFps, recordedConstraintsData))
		{
			this->pushParticlesToGraphicModule(false);

			Storm::IPhysicsManager &physicsMgr = singletonHolder.getSingleton<Storm::IPhysicsManager>();
			physicsMgr.pushConstraintsRecordedFrame(recordedConstraintsData);

			for (const std::unique_ptr<Storm::IBlower> &blowerUPtr : _blowers)
			{
				blowerUPtr->advanceTime(physicsFixedDeltaTime);
			}

			if (autoEndSimulation)
			{
				const float currentPhysicsTime = timeMgr.getCurrentPhysicsElapsedTime();
				hasAutoEndSimulation = currentPhysicsTime > sceneSimulationConfig._endSimulationPhysicsTimeInSeconds;
			}
		}
		else if (autoEndSimulation)
		{
			hasAutoEndSimulation = true;
		}
		else
		{
			LOG_COMMENT << "Simulation has come to an halt because there is no more frame to replay.";
			timeMgr.changeSimulationPauseState();

			_kernelHandler.update(timeMgr.getCurrentPhysicsElapsedTime());
		}

		if (hasAutoEndSimulation)
		{
			timeMgr.quit();
		}
		else
		{
			this->refreshParticleSelection();

			if (autoEndSimulation)
			{
				computeProgression(
					_progressRemainingTime,
					simulationSpeedProfile.getCurrentSpeed(),
					sceneSimulationConfig._endSimulationPhysicsTimeInSeconds,
					timeMgr.getCurrentPhysicsElapsedTime()
				);

				_uiFields->pushField(STORM_PROGRESS_REMAINING_TIME_NAME);
			}
		}

		this->notifyFrameAdvanced();

	} while (true);
}

Storm::ExitCode Storm::SimulatorManager::runSimulation_Internal()
{
	LOG_COMMENT << "Starting simulation loop";

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

	Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();
	Storm::IPhysicsManager &physicsMgr = singletonHolder.getSingleton<Storm::IPhysicsManager>();
	Storm::IThreadManager &threadMgr = singletonHolder.getSingleton<Storm::IThreadManager>();
	Storm::ISpacePartitionerManager &spacePartitionerMgr = singletonHolder.getSingleton<Storm::ISpacePartitionerManager>();

	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
	const Storm::SceneSimulationConfig &sceneSimulationConfig = configMgr.getSceneSimulationConfig();

	assert(!configMgr.isInReplayMode() && "runSimulation_Internal shouldn't be used in replay mode!");

	Storm::IProfilerManager* profilerMgrNullablePtr = configMgr.getGeneralDebugConfig()._profileSimulationSpeed ? singletonHolder.getFacet<Storm::IProfilerManager>() : nullptr;
	const bool hasUI = configMgr.withUI();

	std::vector<Storm::SimulationCallback> tmpSimulationCallback;
	tmpSimulationCallback.reserve(8);

	if (hasUI)
	{
		this->pushParticlesToGraphicModule(true);
	}

	this->refreshParticlePartition(false);

	this->initializePreSimulation();

	const Storm::SceneRecordConfig &sceneRecordConfig = configMgr.getSceneRecordConfig();
	const bool shouldBeRecording = sceneRecordConfig._recordMode == Storm::RecordMode::Record;
	float nextRecordTime = -1.f;
	if (shouldBeRecording)
	{
		this->beginRecord();

		// Record the first frame which is the time 0 (the start).
		const float currentPhysicsTime = timeMgr.getCurrentPhysicsElapsedTime();
		this->pushRecord(currentPhysicsTime, true);
		Storm::ReplaySolver::computeNextRecordTime(nextRecordTime, currentPhysicsTime, sceneRecordConfig._recordFps);
	}

	const bool autoEndSimulation = sceneSimulationConfig._endSimulationPhysicsTimeInSeconds != -1.f;
	bool hasAutoEndSimulation = false;

	bool firstFrame = true;

	const bool noWait = sceneSimulationConfig._simulationNoWait || !hasUI;

	do
	{
		SpeedProfileBalist simulationSpeedProfile{ profilerMgrNullablePtr };

		Storm::TimeWaitResult simulationState = noWait ? timeMgr.getStateNoSyncWait() : timeMgr.waitNextFrame();
		switch (simulationState)
		{
		case Storm::TimeWaitResult::Exit:
			if (hasAutoEndSimulation && profilerMgrNullablePtr)
			{
				LOG_COMMENT <<
					"Simulation average speed was " <<
					profilerMgrNullablePtr->getSpeedProfileAccumulatedTime() / static_cast<float>(_currentFrameNumber);
			}
			return _runExitCode;

		case TimeWaitResult::Pause:
			simulationSpeedProfile.disable();

			// Takes time to process messages that came from other threads.
			threadMgr.processCurrentThreadActions();
			if (sceneSimulationConfig._simulationNoWait)
			{
				// Eh... this is paused so free a little Cpu will you ;)... No need to spin lock even though we said "run as fast as possible" when we're paused...
				std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
			}
			continue;

		case TimeWaitResult::Continue:
		default:
			break;
		}

		_kernelHandler.update(timeMgr.getCurrentPhysicsElapsedTime());

		physicsMgr.notifyIterationStart();
		for (auto &particleSystem : _particleSystem)
		{
			particleSystem.second->onIterationStart();
		}

		// Compute the simulation
		_sphSolver->execute(Storm::IterationParameter{
			._particleSystems = &_particleSystem,
			._kernelLength = this->getKernelLength(),
			._deltaTime = timeMgr.getCurrentPhysicsDeltaTime()
		});

		// Update the particle selector data with the external sum force.
		this->refreshParticleSelection();

		// Push all particle data to the graphic module to be rendered...
		if (hasUI)
		{
			this->pushParticlesToGraphicModule(_currentFrameNumber % 256);
		}

		// Takes time to process messages that came from other threads.
		threadMgr.processCurrentThreadActions();

		float currentPhysicsTime = timeMgr.advanceCurrentPhysicsElapsedTime();

		if (shouldBeRecording)
		{
			if (currentPhysicsTime >= nextRecordTime)
			{
				this->pushRecord(currentPhysicsTime, false);
				Storm::ReplaySolver::computeNextRecordTime(nextRecordTime, currentPhysicsTime, sceneRecordConfig._recordFps);
			}
		}

		hasAutoEndSimulation = autoEndSimulation && currentPhysicsTime > sceneSimulationConfig._endSimulationPhysicsTimeInSeconds;
		if (hasAutoEndSimulation)
		{
			timeMgr.quit();
		}
		else if (autoEndSimulation)
		{
			computeProgression(
				_progressRemainingTime,
				simulationSpeedProfile.getCurrentSpeed(),
				sceneSimulationConfig._endSimulationPhysicsTimeInSeconds,
				timeMgr.getCurrentPhysicsElapsedTime()
			);

			if (hasUI)
			{
				_uiFields->pushField(STORM_PROGRESS_REMAINING_TIME_NAME);
			}
			else if ((_currentFrameNumber % 32) == 0)
			{
				LOG_DEBUG << STORM_PROGRESS_REMAINING_TIME_NAME << " : " << Storm::toStdString(_progressRemainingTime);
			}
		}

		this->notifyFrameAdvanced();

		firstFrame = false;

	} while (true);
}

bool Storm::SimulatorManager::applyCFLIfNeeded(const Storm::SceneSimulationConfig &sceneSimulationConfig)
{
	if (sceneSimulationConfig._computeCFL)
	{
		// 500ms by default seems fine (this time will be the one set if no particle moves)...
		float newDeltaTimeStep = 0.500f;

		/* Compute the max velocity norm during this timestep. */
		float currentStepMaxVelocityNorm = 0.f;

		for (auto &particleSystemPair : _particleSystem)
		{
			if (!particleSystemPair.second->isStatic())
			{
				const std::vector<Storm::Vector3> &velocityField = particleSystemPair.second->getVelocity();
				float maxVelocitySquaredOnParticleSystem = std::max_element(std::execution::par, std::begin(velocityField), std::end(velocityField), [](const Storm::Vector3 &pLeftVelocity, const Storm::Vector3 &pRightVelocity)
				{
					return pLeftVelocity.squaredNorm() < pRightVelocity.squaredNorm();
				})->squaredNorm();

				if (maxVelocitySquaredOnParticleSystem > currentStepMaxVelocityNorm)
				{
					currentStepMaxVelocityNorm = maxVelocitySquaredOnParticleSystem;
				}
			}
		}

		// Since we have a squared velocity norm (optimization reason). Squared root it now.
		if (currentStepMaxVelocityNorm != 0.f)
		{
			currentStepMaxVelocityNorm = std::sqrtf(currentStepMaxVelocityNorm);

			/* Compute the CFL Coefficient */
			const float maxDistanceAllowed = sceneSimulationConfig._particleRadius * 2.f;
			newDeltaTimeStep = computeCFLDistance(sceneSimulationConfig) / currentStepMaxVelocityNorm;
		}
		else if (std::isinf(currentStepMaxVelocityNorm) || std::isnan(currentStepMaxVelocityNorm))
		{
			LOG_WARNING << "Simulation had exploded (at least one particle had an infinite or NaN velocity)!";
		}

		// The physics engine doesn't like when the timestep is below some value...
		constexpr float minDeltaTime = getMinCLFTime();
		if (newDeltaTimeStep < minDeltaTime)
		{
			newDeltaTimeStep = minDeltaTime;
		}

		if (newDeltaTimeStep > sceneSimulationConfig._maxCFLTime)
		{
			newDeltaTimeStep = sceneSimulationConfig._maxCFLTime;
		}

		Storm::ITimeManager &timeMgr = Storm::SingletonHolder::instance().getSingleton<Storm::ITimeManager>();

		/* Apply the new timestep */
		return timeMgr.setCurrentPhysicsDeltaTime(newDeltaTimeStep);
	}

	return false;
}

void Storm::SimulatorManager::initializePreSimulation()
{
	assert(Storm::isSimulationThread() && "This method should only be executed inside the simulation thread!");

	const float k_kernelLength = this->getKernelLength();
	for (auto &particleSystem : _particleSystem)
	{
		Storm::ParticleSystem &pSystem = *particleSystem.second;
		pSystem.initializePreSimulation(_particleSystem, k_kernelLength);
	}

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
	const Storm::SceneSimulationConfig &sceneSimulationConfig = configMgr.getSceneSimulationConfig();

	_sphSolver = Storm::instantiateSPHSolver(Storm::SolverCreationParameter{
		._simulationMode = sceneSimulationConfig._simulationMode,
		._kernelLength = k_kernelLength,
		._particleSystems = &_particleSystem
	});
}

void Storm::SimulatorManager::subIterationStart()
{
	assert(Storm::isSimulationThread() && "This method should only be executed inside the simulation thread!");

	for (auto &particleSystem : _particleSystem)
	{
		particleSystem.second->onSubIterationStart(_particleSystem, _blowers);
	}
}

void Storm::SimulatorManager::revertIteration()
{
	assert(Storm::isSimulationThread() && "This method should only be executed inside the simulation thread!");

	for (auto &particleSystem : _particleSystem)
	{
		particleSystem.second->revertToCurrentTimestep(_blowers);
	}
}

void Storm::SimulatorManager::advanceOneFrame()
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();
	if (timeMgr.getStateNoSyncWait() == Storm::TimeWaitResult::Pause)
	{
		// Unpause to start the current iteration.
		timeMgr.changeSimulationPauseState();
		singletonHolder.getSingleton<Storm::IThreadManager>().executeDefferedOnThread(Storm::ThreadEnumeration::MainThread, [&timeMgr]()
		{
			// Pause again for the iteration to come after the current iteration.
			timeMgr.changeSimulationPauseState();
		});
	}
	else
	{
		LOG_DEBUG_ERROR << "If the simulation is running, it is useless to move to next frame since it will come automatically.";
	}
}

void Storm::SimulatorManager::advanceByFrame(int64_t frameCount)
{
	if (frameCount < 0)
	{
		Storm::throwException<Storm::Exception>("We must advance by a positive frame count (value " + std::to_string(frameCount) + " is invalid)!");
	}

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();

	if (frameCount > 0)
	{
		if (timeMgr.simulationIsPaused())
		{
			timeMgr.changeSimulationPauseState();
		}
		else
		{
			LOG_DEBUG_ERROR << "If the simulation is running, it is useless to move to next frame since it will come automatically.";
			return;
		}
	}
	else if (/*frameCount == 0 &&*/ !timeMgr.simulationIsPaused())
	{
		timeMgr.changeSimulationPauseState();
	}

	singletonHolder.getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::MainThread, [this, toAdvanceFrame = frameCount - 1]()
	{
		_frameAdvanceCount = toAdvanceFrame;
	});
}

void Storm::SimulatorManager::advanceToFrame(int64_t frameNumber)
{
	if (frameNumber < 0)
	{
		Storm::throwException<Storm::Exception>("We must advance to a positive frame number (value " + std::to_string(frameNumber) + " is invalid)!");
	}

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	singletonHolder.getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::MainThread, [this, frameNumber]()
	{
		if (frameNumber < _currentFrameNumber)
		{
			Storm::throwException<Storm::Exception>(
				"The requested frame number (" + std::to_string(frameNumber) + ") to move was already played.\n"
				"We're currently at frame " + std::to_string(_currentFrameNumber) + ".\n"
				"We cannot go back in time!"
			);
		}
		else if (_currentFrameNumber == frameNumber)
		{
			LOG_DEBUG_WARNING << "Requested frame to move to is the same as the current frame we are (" << frameNumber << "). Therefore the operation will be skipped.";
			_frameAdvanceCount = -1;
			return;
		}

		this->advanceByFrame(frameNumber - _currentFrameNumber);
	});
}

void Storm::SimulatorManager::notifyFrameAdvanced()
{
	assert(Storm::isSimulationThread() && "This method should only be executed inside the simulation thread!");

	if (_frameAdvanceCount != -1)
	{
		--_frameAdvanceCount;
		if (_frameAdvanceCount == -1)
		{
			Storm::ITimeManager &timeMgr = Storm::SingletonHolder::instance().getSingleton<Storm::ITimeManager>();
			if (!timeMgr.simulationIsPaused())
			{
				timeMgr.changeSimulationPauseState();
			}
		}
	}

	++_currentFrameNumber;
	_uiFields->pushField(STORM_FRAME_NUMBER_FIELD_NAME);
}

void Storm::SimulatorManager::flushPhysics(const float deltaTime)
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

	Storm::IPhysicsManager &physicsMgr = singletonHolder.getSingleton<Storm::IPhysicsManager>();

	for (auto &particleSystem : _particleSystem)
	{
		Storm::ParticleSystem &pSystem = *particleSystem.second;
		// Only non static rigidbody have a changing physics state managed by PhysX engine.
		if (!pSystem.isFluids() && !pSystem.isStatic())
		{
			physicsMgr.applyLocalForces(particleSystem.first, pSystem.getPositions(), pSystem.getForces());
		}
	}

	const Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();
	physicsMgr.update(timeMgr.getCurrentPhysicsElapsedTime(), deltaTime);

	for (auto &particleSystem : _particleSystem)
	{
		Storm::ParticleSystem &pSystem = *particleSystem.second;
		if (!pSystem.isFluids() && !pSystem.isStatic())
		{
			pSystem.updatePosition(deltaTime, false);
		}
	}
}

void Storm::SimulatorManager::refreshParticlesPosition()
{
	assert(Storm::isSimulationThread() && "This method should only be executed inside the simulation thread!");

	for (auto &particleSystem : _particleSystem)
	{
		Storm::ParticleSystem &pSystem = *particleSystem.second;
		pSystem.updatePosition(0.f, true);
	}
}

void Storm::SimulatorManager::refreshParticleNeighborhood()
{
	assert(Storm::isSimulationThread() && "This method should only be executed inside the simulation thread!");

	this->refreshParticlePartition();

	for (auto &particleSystem : _particleSystem)
	{
		Storm::ParticleSystem &pSystem = *particleSystem.second;
		pSystem.buildNeighborhood(_particleSystem);
	}
}

void Storm::SimulatorManager::onGraphicParticleSettingsChanged()
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();

	singletonHolder.getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::MainThread, [this, &timeMgr]()
	{
		if (timeMgr.simulationIsPaused())
		{
			this->pushParticlesToGraphicModule(true);
		}
	});
}

void Storm::SimulatorManager::addFluidParticleSystem(unsigned int id, std::vector<Storm::Vector3> particlePositions)
{
	const std::size_t initialParticleCount = particlePositions.size();
	LOG_COMMENT << "Creating fluid particle system with " << initialParticleCount << " particles.";

	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const Storm::SceneFluidConfig &sceneFluidConfig = configMgr.getSceneFluidConfig();
	if (sceneFluidConfig._removeParticlesCollidingWithRb)
	{
		LOG_COMMENT << "Removing fluid particles that collide with rigid bodies particles.";

		const Storm::SceneSimulationConfig &sceneSimulationConfig = configMgr.getSceneSimulationConfig();
		removeParticleInsideRbPosition(particlePositions, _particleSystem, sceneSimulationConfig._particleRadius, nullptr);

		const std::size_t currentParticleCount = particlePositions.size();
		LOG_DEBUG << "We removed " << initialParticleCount - currentParticleCount << " particle(s) after checking which collide with existing rigid bodies.";

		if (currentParticleCount == 0)
		{
			LOG_DEBUG_WARNING << "Fluid " << id << " has no particle!";
		}
	}

	addParticleSystemToMap<Storm::FluidParticleSystem>(_particleSystem, id, std::move(particlePositions));

	LOG_DEBUG << "Fluid particle system " << id << " was created and successfully registered in simulator!";
}

void Storm::SimulatorManager::addRigidBodyParticleSystem(unsigned int id, std::vector<Storm::Vector3> particlePositions)
{
	LOG_COMMENT << "Creating rigid body particle system with " << particlePositions.size() << " particles.";

	addParticleSystemToMap<Storm::RigidBodyParticleSystem>(_particleSystem, id, std::move(particlePositions));

	LOG_DEBUG << "Rigid body particle system " << id << " was created and successfully registered in simulator!";
}

void Storm::SimulatorManager::setBlowersStateTime(float blowerStateTime)
{
	for (const auto &blower : _blowers)
	{
		blower->advanceTime(blowerStateTime);
	}
}

void Storm::SimulatorManager::addFluidParticleSystem(Storm::SystemSimulationStateObject &&state)
{
	assert(!Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>().isInReplayMode() && "This method should not be used in replay mode!");


	const std::size_t initialParticleCount = state._positions.size();
	LOG_COMMENT << "Creating fluid particle system with " << initialParticleCount << " particles.";

	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();

	const Storm::SceneSimulationConfig &simulConfig = configMgr.getSceneSimulationConfig();
	const Storm::SceneFluidConfig &sceneFluidConfig = configMgr.getSceneFluidConfig();
	if (simulConfig._shouldRemoveRbCollidingPAtStateFileLoad && sceneFluidConfig._removeParticlesCollidingWithRb)
	{
		LOG_COMMENT << "Removing fluid particles that collide with rigid bodies particles.";

		RemovedIndexesParam removeParam;
		removeParam._shouldConsiderWall = simulConfig._considerRbWallAtCollingingPStateFileLoad;

		const Storm::SceneSimulationConfig &sceneSimulationConfig = configMgr.getSceneSimulationConfig();
		removeParticleInsideRbPosition(state._positions, _particleSystem, sceneSimulationConfig._particleRadius, &removeParam);

		if (!removeParam._outRemovedIndexes.empty())
		{
			std::future<void> removalsFutures[] =
			{
				std::async(std::launch::async, [&state, &removeParam]() { removeFromIndex(state._velocities, removeParam._outRemovedIndexes); }),
				std::async(std::launch::async, [&state, &removeParam]() { removeFromIndex(state._forces, removeParam._outRemovedIndexes); }),
				std::async(std::launch::async, [&state, &removeParam]() { removeFromIndex(state._pressures, removeParam._outRemovedIndexes); }),
				std::async(std::launch::async, [&state, &removeParam]() { removeFromIndex(state._densities, removeParam._outRemovedIndexes); }),
				std::async(std::launch::async, [&state, &removeParam]() { removeFromIndex(state._masses, removeParam._outRemovedIndexes); }),
			};
		}

		LOG_DEBUG << "We removed " << initialParticleCount - state._positions.size() << " particle(s) after checking which collide with existing rigid bodies.";
	}

	Storm::FluidParticleSystem &newPSystem = addParticleSystemToMap<Storm::FluidParticleSystem>(_particleSystem, state._id, std::move(state._positions));
	newPSystem.setVelocity(std::move(state._velocities));
	newPSystem.setForces(std::move(state._forces));
	newPSystem.setPressures(std::move(state._pressures));
	newPSystem.setDensities(std::move(state._densities));
	newPSystem.setMasses(std::move(state._masses));

	LOG_DEBUG << "Fluid particle system " << state._id << " was created and successfully registered in simulator!";
}

void Storm::SimulatorManager::addRigidBodyParticleSystem(Storm::SystemSimulationStateObject &&state)
{
	assert(!Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>().isInReplayMode() && "This method should not be used in replay mode!");

	LOG_COMMENT << "Creating rigid body particle system with " << state._positions.size() << " particles.";

	Storm::RigidBodyParticleSystem &newPSystem = addParticleSystemToMap<Storm::RigidBodyParticleSystem>(_particleSystem, state._id, std::move(state._positions));
	newPSystem.setVelocity(std::move(state._velocities));
	newPSystem.setForces(std::move(state._forces));
	newPSystem.setVolumes(std::move(state._volumes));
	newPSystem.setParticleSystemPosition(state._globalPosition);

	LOG_DEBUG << "Rigid body particle system " << state._id << " was created and successfully registered in simulator!";
}

void Storm::SimulatorManager::addFluidParticleSystem(unsigned int id, const std::size_t particleCount)
{
	LOG_COMMENT << "Creating fluid particle system with " << particleCount << " particles.";

	assert(Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>().isInReplayMode() && "This method is only to be used in replay mode!");
	addParticleSystemToMap<Storm::FluidParticleSystem>(_particleSystem, id, particleCount);

	LOG_DEBUG << "Fluid particle system " << id << " was created and successfully registered in simulator!";
}

void Storm::SimulatorManager::addRigidBodyParticleSystem(unsigned int id, const std::size_t particleCount)
{
	LOG_COMMENT << "Creating rigid body particle system with " << particleCount << " particles.";

	assert(Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>().isInReplayMode() && "This method is only to be used in replay mode!");
	addParticleSystemToMap<Storm::RigidBodyParticleSystem>(_particleSystem, id, particleCount);

	LOG_DEBUG << "Rigid body particle system " << id << " was created and successfully registered in simulator!";
}

std::vector<Storm::Vector3> Storm::SimulatorManager::getParticleSystemPositions(unsigned int id) const
{
	return this->getParticleSystemPositionsReferences(id);
}

const std::vector<Storm::Vector3>& Storm::SimulatorManager::getParticleSystemPositionsReferences(unsigned int id) const
{
	assert(Storm::isSimulationThread() && "this method should only be called from simulation thread!");
	return this->getParticleSystem(id).getPositions();
}

void Storm::SimulatorManager::loadBlower(const Storm::SceneBlowerConfig &blowerConfig)
{
#define STORM_XMACRO_GENERATE_ELEMENTARY_BLOWER(BlowerTypeName, BlowerTypeXmlName, EffectAreaType, MeshMakerType) \
case Storm::BlowerType::BlowerTypeName: appendNewBlower<Storm::BlowerType::BlowerTypeName, Storm::EffectAreaType>(_blowers, blowerConfig); break;

		switch (blowerConfig._blowerType)
		{
			STORM_XMACRO_GENERATE_BLOWERS_CODE;

		default:
			Storm::throwException<Storm::Exception>("Unhandled Blower Type creation requested! Value was " + std::to_string(static_cast<int>(blowerConfig._blowerType)));
		}

#undef STORM_XMACRO_GENERATE_ELEMENTARY_BLOWER
}

void Storm::SimulatorManager::tweekBlowerEnabling()
{
	for (auto &blower : _blowers)
	{
		blower->tweakEnabling();
	}
}

void Storm::SimulatorManager::advanceBlowersTime(const float deltaTime)
{
	for (const std::unique_ptr<Storm::IBlower> &blowerUPtr : _blowers)
	{
		blowerUPtr->advanceTime(deltaTime);
	}
}

void Storm::SimulatorManager::tweekRaycastEnabling()
{
	if (_raycastFlag == Storm::RaycastEnablingFlag::Disabled)
	{
		STORM_ADD_BIT_ENABLED(_raycastFlag, Storm::RaycastEnablingFlag::DynamicRigidBodies);
		LOG_DEBUG << "Enabling raycast on dynamic rigid body";
	}
	else if(STORM_IS_BIT_ENABLED(_raycastFlag, Storm::RaycastEnablingFlag::Fluids))
	{
		_raycastFlag = Storm::RaycastEnablingFlag::Disabled;
		LOG_DEBUG << "Disabling raycast";
	}
	else
	{
		STORM_ADD_BIT_ENABLED(_raycastFlag, Storm::RaycastEnablingFlag::Fluids);
		LOG_DEBUG << "Enabling raycast on fluids";
	}
}

float Storm::SimulatorManager::getKernelLength() const
{
	return _kernelHandler.getKernelValue();
}

void Storm::SimulatorManager::pushParticlesToGraphicModule(bool ignoreDirty) const
{
	assert(Storm::isSimulationThread() && "This method should only be executed inside the simulation thread!");

	Storm::IGraphicsManager &graphicMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IGraphicsManager>();

	if (_particleSelector.hasSelectedParticle())
	{
		if (auto found = _particleSystem.find(_particleSelector.getSelectedParticleSystemId()); found != std::end(_particleSystem))
		{
			const Storm::ParticleSystem &selectedParticleSystem = *found->second;

			const std::size_t selectedParticleIndex = _particleSelector.getSelectedParticleIndex();

			const Storm::Vector3 &selectedParticlePosition = selectedParticleSystem.getPositions()[selectedParticleIndex];
			graphicMgr.pushParticleSelectionForceData(_particleSelector.getSelectedVectorPosition(selectedParticlePosition), _particleSelector.getSelectedVectorToDisplay());
		}
	}
	
	const auto pushActionLambda = [&graphicMgr, ignoreDirty](const auto &particleSystemPair)
	{
		const Storm::ParticleSystem &currentParticleSystem = *particleSystemPair.second;
		if (ignoreDirty || currentParticleSystem.isDirty() || currentParticleSystem.isFluids())
		{
			Storm::PushedParticleSystemDataParameter param{
				._particleSystemId = particleSystemPair.first,
				._positionsData = &currentParticleSystem.getPositions(),
				._velocityData = &currentParticleSystem.getVelocity(),
				._isFluids = currentParticleSystem.isFluids(),
				._isWall = currentParticleSystem.isWall()
			};

			if (param._isFluids)
			{
				const Storm::FluidParticleSystem &currentPSystemAsFluid = static_cast<const Storm::FluidParticleSystem &>(currentParticleSystem);
				param._pressureData = &currentPSystemAsFluid.getPressures();
				param._densityData = &currentPSystemAsFluid.getDensities();
			}
			else
			{
				param._pressureData = nullptr;
				param._densityData = nullptr;
			}

			graphicMgr.pushParticlesData(param);
		}
	};

	Storm::runParallel(_particleSystem, pushActionLambda);
}

void Storm::SimulatorManager::cycleSelectedParticleDisplayMode()
{
	assert(Storm::isSimulationThread() && "This method should only be executed inside the simulation thread!");

	_particleSelector.cycleParticleSelectionDisplayMode();

	if (_particleSelector.hasSelectedParticle())
	{
		const Storm::ITimeManager &timeMgr = Storm::SingletonHolder::instance().getSingleton<Storm::ITimeManager>();
		if (timeMgr.getStateNoSyncWait() == Storm::TimeWaitResult::Pause)
		{
			this->pushParticlesToGraphicModule(true);
		}
	}
}

void Storm::SimulatorManager::refreshParticleSelection()
{
	assert(Storm::isSimulationThread() && "This method should only be executed inside the simulation thread!");

	if (_particleSelector.hasSelectedParticle())
	{
		if (auto found = _particleSystem.find(_particleSelector.getSelectedParticleSystemId()); found != std::end(_particleSystem))
		{
			const Storm::ParticleSystem &pSystem = *found->second;

			const std::size_t selectedParticleIndex = _particleSelector.getSelectedParticleIndex();
			_particleSelector.setSelectedParticleVelocity(pSystem.getVelocity()[selectedParticleIndex]);
			_particleSelector.setSelectedParticlePressureForce(pSystem.getTemporaryPressureForces()[selectedParticleIndex]);
			_particleSelector.setSelectedParticleViscosityForce(pSystem.getTemporaryViscosityForces()[selectedParticleIndex]);
			_particleSelector.setSelectedParticleSumForce(pSystem.getForces()[selectedParticleIndex]);

			if (!pSystem.isFluids())
			{
				const Storm::RigidBodyParticleSystem &pSystemAsRb = static_cast<const Storm::RigidBodyParticleSystem &>(pSystem);
				_particleSelector.setRbPosition(pSystemAsRb.getRbPosition());
				_particleSelector.setRbTotalForce(pSystemAsRb.getRbTotalForce());
			}
		}
	}
}

void Storm::SimulatorManager::requestCycleColoredSetting()
{
	assert(Storm::isSimulationThread() && "This method should only be executed inside the simulation thread!");

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	singletonHolder.getSingleton<Storm::IGraphicsManager>().cycleColoredSetting();
	
	if (singletonHolder.getSingleton<Storm::ITimeManager>().simulationIsPaused())
	{
		this->pushParticlesToGraphicModule(true);
	}
}

void Storm::SimulatorManager::exitWithCode(const Storm::ExitCode code)
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	singletonHolder.getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::MainThread, [this, code, &singletonHolder]()
	{
		_runExitCode = code;
		singletonHolder.getSingleton<Storm::ITimeManager>().quit();
	});
}

void Storm::SimulatorManager::beginRecord() const
{
	assert(Storm::isSimulationThread() && "This method should only be executed inside the simulation thread!");

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

	const Storm::SceneRecordConfig &sceneRecordConfig = singletonHolder.getSingleton<Storm::IConfigManager>().getSceneRecordConfig();

	Storm::SerializeRecordHeader recordHeader;
	recordHeader._recordFrameRate = sceneRecordConfig._recordFps;

	recordHeader._particleSystemLayouts.reserve(_particleSystem.size());
	for (const auto &particleSystemPair : _particleSystem)
	{
		const Storm::ParticleSystem &pSystemRef = *particleSystemPair.second;

		Storm::SerializeParticleSystemLayout &pSystemLayoutRef = recordHeader._particleSystemLayouts.emplace_back();

		pSystemLayoutRef._particleSystemId = particleSystemPair.first;
		pSystemLayoutRef._particlesCount = pSystemRef.getParticleCount();
		pSystemLayoutRef._isFluid = pSystemRef.isFluids();
		pSystemLayoutRef._isStatic = pSystemRef.isStatic();
	}

	const Storm::IPhysicsManager &physicsMgr = singletonHolder.getSingleton<Storm::IPhysicsManager>();
	physicsMgr.getConstraintsRecordLayoutData(recordHeader._contraintLayouts);

	Storm::ISerializerManager &serializerMgr = singletonHolder.getSingleton<Storm::ISerializerManager>();
	serializerMgr.beginRecord(std::move(recordHeader));
}

void Storm::SimulatorManager::pushRecord(float currentPhysicsTime, bool pushStatics) const
{
	assert(Storm::isSimulationThread() && "This method should only be executed inside the simulation thread!");

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::SceneRecordConfig &sceneRecordConfig = singletonHolder.getSingleton<Storm::IConfigManager>().getSceneRecordConfig();

	Storm::SerializeRecordPendingData currentFrameData;
	currentFrameData._physicsTime = currentPhysicsTime;

	for (const auto &particleSystemPair : _particleSystem)
	{
		const Storm::ParticleSystem &pSystemRef = *particleSystemPair.second;

		if (pushStatics || !pSystemRef.isStatic())
		{
			Storm::SerializeRecordParticleSystemData &framePSystemElementData = currentFrameData._particleSystemElements.emplace_back();

			framePSystemElementData._systemId = particleSystemPair.first;

			if (pSystemRef.isFluids())
			{
				const Storm::FluidParticleSystem &pSystemRefAsFluid = static_cast<const Storm::FluidParticleSystem &>(pSystemRef);
				framePSystemElementData._densities = pSystemRefAsFluid.getDensities();
				framePSystemElementData._pressures = pSystemRefAsFluid.getPressures();
			}
			else
			{
				const Storm::RigidBodyParticleSystem &pSystemRefAsRb = static_cast<const Storm::RigidBodyParticleSystem &>(pSystemRef);
				framePSystemElementData._pSystemPosition = pSystemRefAsRb.getRbPosition();
				framePSystemElementData._pSystemGlobalForce = pSystemRefAsRb.getRbTotalForce();
				framePSystemElementData._volumes = pSystemRefAsRb.getVolumes();
			}

			framePSystemElementData._positions = pSystemRef.getPositions();
			framePSystemElementData._velocities = pSystemRef.getVelocity();
			framePSystemElementData._forces = pSystemRef.getForces();
			framePSystemElementData._pressureComponentforces = pSystemRef.getTemporaryPressureForces();
			framePSystemElementData._viscosityComponentforces = pSystemRef.getTemporaryViscosityForces();
		}
	}

	const Storm::IPhysicsManager &physicsMgr = singletonHolder.getSingleton<Storm::IPhysicsManager>();
	physicsMgr.getConstraintsRecordFrameData(currentFrameData._constraintElements);

	Storm::ISerializerManager &serializerMgr = singletonHolder.getSingleton<Storm::ISerializerManager>();
	serializerMgr.recordFrame(std::move(currentFrameData));
}

void Storm::SimulatorManager::applyReplayFrame(Storm::SerializeRecordPendingData &frame, const float replayFps, bool pushParallel /*= true*/)
{
	assert(Storm::isSimulationThread() && "This method should only be executed inside the simulation thread!");

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

	Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();
	timeMgr.setCurrentPhysicsElapsedTime(frame._physicsTime);

	if (timeMgr.getExpectedFrameFPS() == replayFps)
	{
		Storm::ReplaySolver::transferFrameToParticleSystem_move(_particleSystem, frame);
	}
	else
	{
		// We need the frameBefore afterward, therefore we will make copy...
		Storm::ReplaySolver::transferFrameToParticleSystem_copy(_particleSystem, frame);
	}

	this->refreshParticleSelection();

	this->pushParticlesToGraphicModule(true);

	Storm::IPhysicsManager &physicsMgr = singletonHolder.getSingleton<Storm::IPhysicsManager>();
	physicsMgr.pushConstraintsRecordedFrame(frame._constraintElements);
}

void Storm::SimulatorManager::resetReplay()
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	singletonHolder.getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::MainThread, [this]()
	{
		this->resetReplay();
	});
}

void Storm::SimulatorManager::resetReplay_SimulationThread()
{
	assert(Storm::isSimulationThread() && "This method should only be executed inside the simulation thread!");

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
	if (configMgr.isInReplayMode())
	{
		Storm::ISerializerManager &serializerMgr = singletonHolder.getSingleton<Storm::ISerializerManager>();
		if (serializerMgr.resetReplay())
		{
			Storm::SerializeRecordPendingData &frameBefore = *_frameBefore;
			if (serializerMgr.obtainNextFrame(frameBefore))
			{
				const Storm::SceneRecordConfig &sceneRecordConfig = configMgr.getSceneRecordConfig();
				this->applyReplayFrame(frameBefore, sceneRecordConfig._recordFps, false);
				
				_reinitFrameAfter = true;

				LOG_DEBUG << "Replay successfully reset to its starting point.";
			}
			else
			{
				LOG_ERROR << "Error occurred when obtaining the next frame.";
			}
		}
		else
		{
			LOG_ERROR << "Reset replay returned an error!";
		}
	}
	else
	{
		LOG_WARNING << "Simulation reset is only possible in Replay mode.";
	}
}

void Storm::SimulatorManager::saveSimulationState() const
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	singletonHolder.getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::MainThread, [this, &singletonHolder]()
	{
		assert(Storm::isSimulationThread() && "This piece of code should only be executed inside the simulation thread!");

		const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
		const Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();

		Storm::StateSavingOrders order;

		// Settings
		order._settings._filePath = configMgr.getTemporaryPath();
		order._settings._filePath /= "States";
		order._settings._filePath /= configMgr.getSceneName();
		order._settings._filePath /= "state_" + std::to_string(timeMgr.getCurrentPhysicsElapsedTime()) + ".stState";

		order._settings._overwrite = false;
		order._settings._autoPathIfNoOwerwrite = true;

		// Pre-saving step
		bool isReplayMode = configMgr.isInReplayMode();
		for (auto &pSystemPair : _particleSystem)
		{
			pSystemPair.second->prepareSaving(isReplayMode);
		}

		// State update
		Storm::StateSaverHelper::saveIntoState(*order._simulationState, _particleSystem);

		Storm::ISerializerManager &serializerMgr = singletonHolder.getSingleton<Storm::ISerializerManager>();
		serializerMgr.saveState(std::move(order));
	});
}

void Storm::SimulatorManager::refreshParticlePartition(bool ignoreStatics /*= true*/) const
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	Storm::ISpacePartitionerManager &spacePartitionerMgr = singletonHolder.getSingleton<Storm::ISpacePartitionerManager>();

	spacePartitionerMgr.clearSpaceReorderingNoStatic();
	if (ignoreStatics)
	{
		for (auto &particleSystem : _particleSystem)
		{
			// We don't need to regenerate statics rigid bodies.
			Storm::ParticleSystem &pSystem = *particleSystem.second;
			if (!pSystem.isStatic())
			{
				spacePartitionerMgr.computeSpaceReordering(
					pSystem.getPositions(),
					pSystem.isFluids() ? Storm::PartitionSelection::Fluid : Storm::PartitionSelection::DynamicRigidBody,
					pSystem.getId()
				);
			}
		}
	}
	else
	{
		spacePartitionerMgr.clearSpaceReorderingForPartition(Storm::PartitionSelection::StaticRigidBody);
		for (auto &particleSystem : _particleSystem)
		{
			Storm::ParticleSystem &pSystem = *particleSystem.second;

			Storm::PartitionSelection selection;
			if (pSystem.isFluids())
			{
				selection = Storm::PartitionSelection::Fluid;
			}
			else if (pSystem.isStatic())
			{
				selection = Storm::PartitionSelection::StaticRigidBody;
			}
			else
			{
				selection = Storm::PartitionSelection::DynamicRigidBody;
			}

			spacePartitionerMgr.computeSpaceReordering(pSystem.getPositions(), selection, pSystem.getId());
		}
	}
}

Storm::ParticleSystem& Storm::SimulatorManager::getParticleSystem(unsigned int id)
{
	if (const auto foundParticleSystem = _particleSystem.find(id); foundParticleSystem != std::end(_particleSystem))
	{
		return *foundParticleSystem->second;
	}
	else
	{
		Storm::throwException<Storm::Exception>("Particle system with id " + std::to_string(id) + " is unknown!");
	}
}

void Storm::SimulatorManager::printFluidParticleData() const
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	singletonHolder.getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::MainThread, [this, &singletonHolder]()
	{
		static int s_id = 0;
		std::filesystem::path filePath = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>().getTemporaryPath();
		filePath /= "Debug";
		filePath /= "fluidData_" + std::to_string(s_id++) + ".txt";

		std::filesystem::create_directories(filePath.parent_path());

		std::ofstream file{ filePath.string() };

		for (const auto &particleSystemPair : _particleSystem)
		{
			const Storm::ParticleSystem &currentPSystem = *particleSystemPair.second;
			if (currentPSystem.isFluids())
			{
				printToStream<false>(file, currentPSystem.getPositions(), "Position");
				printToStream<true>(file, currentPSystem.getVelocity(), "Velocity");
				printToStream<true>(file, currentPSystem.getForces(), "Force");
				printToStream<true>(file, currentPSystem.getNeighborhoodArrays(), "Neighborhood");
			}
		}
	});
}

const Storm::ParticleSystem& Storm::SimulatorManager::getParticleSystem(unsigned int id) const
{
	if (const auto foundParticleSystem = _particleSystem.find(id); foundParticleSystem != std::end(_particleSystem))
	{
		return *foundParticleSystem->second;
	}
	else
	{
		Storm::throwException<Storm::Exception>("Particle system with id " + std::to_string(id) + " is unknown!");
	}
}

void Storm::SimulatorManager::executeAllForcesCheck()
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	Storm::IPhysicsManager &physicsMgr = singletonHolder.getSingleton<Storm::IPhysicsManager>();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

	const Storm::SceneSimulationConfig &sceneSimulationConfig = configMgr.getSceneSimulationConfig();
	const Storm::Vector3 &gravityAccel = sceneSimulationConfig._gravity;

	// Ensure that, from the momentum conservation law, all forces present in the domain (which is an isolated system) comes to zero
	// (note that it shouldn't be true if the system is not in an equilibrium state because forces do work to convert potential energy into kinetic energy then.
	// If there is work, then the system is not in a linear uniform movement).
	// I.e to illustrate my explanation : 
	// In the first second of a dam break type simulation, the particles are falling, and the ones on the floor will move to the area where there is no particle.
	// If the particle column are in the right of the domain, then the particle will be pushed to the right, resulting in a global force that is down right
	// (down because of gravity that make particles fall, and right because when falling, particle will push the other particles down,
	// but the one that cannot be pushed down because of the floor, will be pushed either to the right or left (front and bottom too).
	// Except that the wall at the extreme right will make a reaction force that prevent the particle pushed to the right to pass through the wall,
	// therefore the global move will be to the left.
	//
	// But even with this fact, I'm not sure about the real physical state of the engine because what I'm saying is only if we have a perfect system,
	// except that we simulate imperfect system with viscosity, therefore some energy is lost with the friction it implies.
	std::atomic<float> xSum = 0.f;
	std::atomic<float> ySum = 0.f;
	std::atomic<float> zSum = 0.f;
	for (const auto &particleSystemPair : _particleSystem)
	{
		const Storm::ParticleSystem &currentPSystem = *particleSystemPair.second;

		Storm::runParallel(currentPSystem.getForces(), [&xSum, &ySum, &zSum](const Storm::Vector3 &currentPForce)
		{
			xSum += currentPForce.x();
			ySum += currentPForce.y();
			zSum += currentPForce.z();
		});

		if (!currentPSystem.isFluids() && !currentPSystem.isStatic())
		{
			// For rigid bodies, some forces like the gravity or the constraint are handled by the physics engine, therefore we must query those. 
			const Storm::Vector3 additionalForces = physicsMgr.getPhysicalForceOnPhysicalBody(currentPSystem.getId());

			xSum += additionalForces.x();
			ySum += additionalForces.y();
			zSum += additionalForces.z();
		}
	}

	const Storm::Vector3 allForceSum{ xSum, ySum, zSum };
	constexpr float epsilon = 0.0000001f;
	if (std::fabs(allForceSum.x()) > epsilon || std::fabs(allForceSum.y()) > epsilon || std::fabs(allForceSum.z()) > epsilon)
	{
		LOG_ERROR << "All force sum are not equal to 0! Resulting force is " << allForceSum;
	}
}

void Storm::SimulatorManager::printRigidBodyMoment(const unsigned int id) const
{
	if (const auto found = std::find_if(std::begin(_particleSystem), std::end(_particleSystem), [id](const auto &currentPSystemPair)
	{
		return currentPSystemPair.second->getId() == id;
	}); found != std::end(_particleSystem))
	{
		const Storm::ParticleSystem &currentPSystem = *found->second;
		if (currentPSystem.isFluids() || currentPSystem.isStatic())
		{
			Storm::throwException<Storm::Exception>("Particle system with id " + std::to_string(id) + " exists but is not a dynamic rigid body!");
		}

		const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

		const Storm::IPhysicsManager &physicsMgr = singletonHolder.getSingleton<Storm::IPhysicsManager>();
		Storm::IThreadManager &threadMgr = singletonHolder.getSingleton<Storm::IThreadManager>();

		threadMgr.executeOnThread(Storm::ThreadEnumeration::MainThread, [this, &currentPSystem, &physicsMgr, id]()
		{
			Storm::Vector3 rbPos;
			Storm::Quaternion rbRot;
			physicsMgr.getMeshTransform(id, rbPos, rbRot);

			const std::vector<Storm::Vector3> &rbParticlePositions = currentPSystem.getPositions();
			const std::vector<Storm::Vector3> &rbParticleForces = currentPSystem.getForces();

			Storm::Vector3 moment = Storm::Vector3::Zero();
			const std::size_t particleCount = rbParticlePositions.size();

			assert(particleCount == rbParticleForces.size() && "Mismatch detected between particle and forces count!");

			for (std::size_t iter = 0; iter < particleCount; ++iter)
			{
				moment += (rbParticlePositions[iter] - rbPos).cross(rbParticleForces[iter]);
			}

			LOG_ALWAYS << "Moment of rigid body " << id << " is " << moment << " (Note that we have considered only the pressure and viscosity forces).";
		});
	}
	else
	{
		Storm::throwException<Storm::Exception>("Cannot find the rigid body with id " + std::to_string(id) + "!");
	}
}
