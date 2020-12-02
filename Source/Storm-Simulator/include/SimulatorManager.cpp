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

#include "GeneralSimulationData.h"
#include "FluidData.h"
#include "RecordConfigData.h"

#include "KernelMode.h"
#include "RecordMode.h"
#include "ReplaySolver.h"

#include "PartitionSelection.h"

#include "SemiImplicitEulerSolver.h"
#include "Kernel.h"

#include "SPHBaseSolver.h"

#include "SpecialKey.h"

#include "RunnerHelper.h"
#include "Vector3Utils.h"
#include "BoundingBox.h"

#include "BlowerDef.h"
#include "BlowerType.h"
#include "BlowerData.h"
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

#include "ExitCode.h"

#include "UIField.h"
#include "UIFieldContainer.h"

#include "SolverCreationParameter.h"
#include "IterationParameter.h"

#include <fstream>

#define STORM_PROGRESS_REMAINING_TIME_NAME "Remaining time"


namespace
{
	template<class ParticleSystemType, class MapType, class ... Args>
	void addParticleSystemToMap(MapType &map, unsigned int particleSystemId, Args &&... args)
	{
		std::unique_ptr<Storm::ParticleSystem> particleSystemPtr = std::make_unique<ParticleSystemType>(particleSystemId, std::forward<Args>(args)...);
		map[particleSystemId] = std::move(particleSystemPtr);
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
				inOutResult += Storm::toStdString<PolicyType>(neighbor._vectToParticleNorm);
				inOutResult += "; xij=";
				inOutResult += Storm::toStdString<PolicyType>(neighbor._positionDifferenceVector);
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
	void appendNewBlower(std::vector<std::unique_ptr<Storm::IBlower>> &inOutBlowerContainer, const Storm::BlowerData &blowerDataConfig)
	{
		std::string_view blowerIntroMsg;
		if constexpr (type != Storm::BlowerType::PulseExplosionSphere)
		{
			if (blowerDataConfig._fadeInTimeInSeconds > 0.f && blowerDataConfig._fadeOutTimeInSeconds > 0.f)
			{
				inOutBlowerContainer.emplace_back(std::make_unique<Storm::Blower<type, BlowerEffectArea, Storm::FadeInOutTimeHandler, BlowerCallbacks>>(blowerDataConfig));
				blowerIntroMsg = "Blower with fadeIn and fadeOut feature created.\n";
			}
			else if (blowerDataConfig._fadeInTimeInSeconds > 0.f)
			{
				inOutBlowerContainer.emplace_back(std::make_unique<Storm::Blower<type, BlowerEffectArea, Storm::FadeInTimeHandler, BlowerCallbacks>>(blowerDataConfig));
				blowerIntroMsg = "Blower with fadeIn only feature created.\n";
			}
			else if (blowerDataConfig._fadeOutTimeInSeconds > 0.f)
			{
				inOutBlowerContainer.emplace_back(std::make_unique<Storm::Blower<type, BlowerEffectArea, Storm::FadeOutTimeHandler, BlowerCallbacks>>(blowerDataConfig));
				blowerIntroMsg = "Blower with fadeOut only feature created.\n";
			}
			else
			{
				inOutBlowerContainer.emplace_back(std::make_unique<Storm::Blower<type, BlowerEffectArea, Storm::BlowerTimeHandlerBase, BlowerCallbacks>>(blowerDataConfig));
				blowerIntroMsg = "Blower without fadeIn or fadeOut only feature created.\n";
			}
		}
		else
		{
			inOutBlowerContainer.emplace_back(std::make_unique<Storm::Blower<Storm::BlowerType::PulseExplosionSphere, BlowerEffectArea, Storm::BlowerPulseTimeHandler, BlowerCallbacks>>(blowerDataConfig));
			blowerIntroMsg = "Explosion Blower Effect created.\n";
		}

		LOG_DEBUG << blowerIntroMsg <<
			"The blower is placed at " << blowerDataConfig._blowerPosition <<
			", has a dimension of " << blowerDataConfig._blowerDimension <<
			" and a force of " << blowerDataConfig._blowerForce <<
			" will start at " << blowerDataConfig._startTimeInSeconds << "s.";
	}

	inline Storm::Vector3 computeInternalBoundingBoxCorner(const Storm::Vector3 &externalBoundingBoxCorner, const Storm::Vector3 &externalBoundingBoxTranslation, const float coeff)
	{
		// First we need to center the bounding box around { 0, 0, 0 } or it won't scale correctly
		// (the scale will translate/shift the internal bounding box, or the coeff will be applied differently depending on how far we are from 0).
		// Second, we apply back the shift once the scaling was done.
		return (externalBoundingBoxCorner - externalBoundingBoxTranslation) * coeff + externalBoundingBoxTranslation;
	}

	void removeParticleInsideRbPosition(std::vector<Storm::Vector3> &inOutParticlePositions, const Storm::ParticleSystemContainer &allParticleSystem, const float particleRadius)
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
		}

		for (std::size_t toRemove = std::end(inOutParticlePositions) - thresholdToEliminate; toRemove > 0; --toRemove)
		{
			inOutParticlePositions.pop_back();
		}
	}

	float computeCFLDistance(const Storm::GeneralSimulationData &generalConfig)
	{
		const float maxDistanceAllowed = generalConfig._particleRadius * 2.f;
		return generalConfig._cflCoeff * maxDistanceAllowed;
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
	_raycastEnabled{ false },
	_runExitCode{ Storm::ExitCode::k_success },
	_reinitFrameAfter{ false },
	_progressRemainingTime{ STORM_TEXT("N/A") },
	_uiFields{ std::make_unique<Storm::UIFieldContainer>() }
{

}

Storm::SimulatorManager::~SimulatorManager() = default;

void Storm::SimulatorManager::initialize_Implementation()
{
	LOG_COMMENT << "Initializing the simulator";

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

	/* initialize the Selector */

	_particleSelector.initialize();

	_kernelHandler.initialize();

	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
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

		const Storm::RecordConfigData &recordConfig = configMgr.getRecordConfigData();
		this->applyReplayFrame(frameBefore, recordConfig._recordFps);
	}
	else
	{
		/* Initialize kernels */

		Storm::initializeKernels(this->getKernelLength());
	}

	/* Initialize inputs */

	Storm::IInputManager &inputMgr = singletonHolder.getSingleton<Storm::IInputManager>();
	inputMgr.bindKey(Storm::SpecialKey::KC_F1, [this]() { this->printFluidParticleData(); });
	inputMgr.bindKey(Storm::SpecialKey::KC_E, [this]() { this->tweekBlowerEnabling(); });
	inputMgr.bindKey(Storm::SpecialKey::KC_R, [this]() { this->tweekRaycastEnabling(); });
	inputMgr.bindKey(Storm::SpecialKey::KC_Y, [this]() { this->cycleSelectedParticleDisplayMode(); });
	inputMgr.bindKey(Storm::SpecialKey::KC_I, [this]() { this->resetReplay(); });
	inputMgr.bindKey(Storm::SpecialKey::KC_C, [this]() { this->executeAllForcesCheck(); });

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
		if (_raycastEnabled)
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

				if (hasMadeSelectionChanges && timeMgr.getStateNoSyncWait() == Storm::TimeWaitResult::Pause)
				{
					this->pushParticlesToGraphicModule(true, false);
				}
			} }.addPartitionFlag(Storm::PartitionSelection::DynamicRigidBody)
			   .firstHitOnly())
			);
		}
	});

	inputMgr.bindMouseMiddleClick([this, &raycastMgr, &singletonHolder, isReplayMode](int xPos, int yPos, int width, int height)
	{
		if (_raycastEnabled)
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

	const bool autoEndSimulation = configMgr.getGeneralSimulationData()._endSimulationPhysicsTimeInSeconds != -1.f;
	if (autoEndSimulation)
	{
		(*_uiFields)
			.bindField(STORM_PROGRESS_REMAINING_TIME_NAME, _progressRemainingTime)
			;
	}
}

Storm::ExitCode Storm::SimulatorManager::run()
{
	switch (Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>().getRecordConfigData()._recordMode)
	{
	case Storm::RecordMode::None:
	case Storm::RecordMode::Record:
		return this->runSimulation_Internal();

	case Storm::RecordMode::Replay:
		return this->runReplay_Internal();

	default:
		__assume(false);
		Storm::throwException<std::exception>("Unknown record mode!");
	}
}

Storm::ExitCode Storm::SimulatorManager::runReplay_Internal()
{
	LOG_COMMENT << "Starting replay loop";

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
	const Storm::GeneralSimulationData &generalSimulationConfigData = configMgr.getGeneralSimulationData();

	Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();
	Storm::IThreadManager &threadMgr = singletonHolder.getSingleton<Storm::IThreadManager>();
	Storm::IProfilerManager* profilerMgrNullablePtr = configMgr.getShouldProfileSimulationSpeed() ? singletonHolder.getFacet<Storm::IProfilerManager>() : nullptr;

	const Storm::RecordConfigData &recordConfig = configMgr.getRecordConfigData();
	Storm::ISerializerManager &serializerMgr = singletonHolder.getSingleton<Storm::ISerializerManager>();

	const bool autoEndSimulation = generalSimulationConfigData._endSimulationPhysicsTimeInSeconds != -1.f;
	bool hasAutoEndSimulation = false;

	unsigned int forcedPushFrameIterator = 0;

	Storm::SerializeRecordPendingData &frameBefore = *_frameBefore;
	Storm::SerializeRecordPendingData frameAfter;

	this->refreshParticlePartition(false);

	float expectedReplayFps;
	if (recordConfig._replayRealTime)
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
		if (recordConfig._replayRealTime)
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
			simulationState = generalSimulationConfigData._simulationNoWait ? timeMgr.getStateNoSyncWait() : timeMgr.waitNextFrame();
		}

		switch (simulationState)
		{
		case Storm::TimeWaitResult::Exit:
			if (hasAutoEndSimulation && profilerMgrNullablePtr)
			{
				LOG_COMMENT <<
					"Simulation average speed was " <<
					profilerMgrNullablePtr->getSpeedProfileAccumulatedTime() / static_cast<float>(forcedPushFrameIterator);
			}

			return _runExitCode;

		case TimeWaitResult::Pause:
			simulationSpeedProfile.disable();

			// Takes time to process messages that came from other threads.
			threadMgr.processCurrentThreadActions();
			
			if (generalSimulationConfigData._simulationNoWait)
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
				hasAutoEndSimulation = currentPhysicsTime > generalSimulationConfigData._endSimulationPhysicsTimeInSeconds;
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
					generalSimulationConfigData._endSimulationPhysicsTimeInSeconds,
					timeMgr.getCurrentPhysicsElapsedTime()
				);

				_uiFields->pushField(STORM_PROGRESS_REMAINING_TIME_NAME);
			}
		}

		++forcedPushFrameIterator;

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
	const Storm::GeneralSimulationData &generalSimulationConfigData = configMgr.getGeneralSimulationData();

	assert(!configMgr.isInReplayMode() && "runSimulation_Internal shouldn't be used in replay mode!");

	Storm::IProfilerManager* profilerMgrNullablePtr = configMgr.getShouldProfileSimulationSpeed() ? singletonHolder.getFacet<Storm::IProfilerManager>() : nullptr;
	const bool hasUI = configMgr.withUI();

	std::vector<Storm::SimulationCallback> tmpSimulationCallback;
	tmpSimulationCallback.reserve(8);

	if (hasUI)
	{
		this->pushParticlesToGraphicModule(true);
	}

	this->refreshParticlePartition(false);

	this->initializePreSimulation();

	const Storm::RecordConfigData &recordConfig = configMgr.getRecordConfigData();
	const bool shouldBeRecording = recordConfig._recordMode == Storm::RecordMode::Record;
	float nextRecordTime = -1.f;
	if (shouldBeRecording)
	{
		this->beginRecord();

		// Record the first frame which is the time 0 (the start).
		const float currentPhysicsTime = timeMgr.getCurrentPhysicsElapsedTime();
		this->pushRecord(currentPhysicsTime, true);
		Storm::ReplaySolver::computeNextRecordTime(nextRecordTime, currentPhysicsTime, recordConfig._recordFps);
	}

	const bool autoEndSimulation = generalSimulationConfigData._endSimulationPhysicsTimeInSeconds != -1.f;
	bool hasAutoEndSimulation = false;

	unsigned int forcedPushFrameIterator = 0;

	bool firstFrame = true;

	const bool noWait = generalSimulationConfigData._simulationNoWait || !hasUI;

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
					profilerMgrNullablePtr->getSpeedProfileAccumulatedTime() / static_cast<float>(forcedPushFrameIterator);
			}
			return _runExitCode;

		case TimeWaitResult::Pause:
			simulationSpeedProfile.disable();

			// Takes time to process messages that came from other threads.
			threadMgr.processCurrentThreadActions();
			if (generalSimulationConfigData._simulationNoWait)
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
			this->pushParticlesToGraphicModule(forcedPushFrameIterator % 256);
		}

		// Takes time to process messages that came from other threads.
		threadMgr.processCurrentThreadActions();

		float currentPhysicsTime = timeMgr.advanceCurrentPhysicsElapsedTime();

		if (shouldBeRecording)
		{
			if (currentPhysicsTime >= nextRecordTime)
			{
				this->pushRecord(currentPhysicsTime, false);
				Storm::ReplaySolver::computeNextRecordTime(nextRecordTime, currentPhysicsTime, recordConfig._recordFps);
			}
		}

		hasAutoEndSimulation = autoEndSimulation && currentPhysicsTime > generalSimulationConfigData._endSimulationPhysicsTimeInSeconds;
		if (hasAutoEndSimulation)
		{
			timeMgr.quit();
		}
		else if (autoEndSimulation)
		{
			computeProgression(
				_progressRemainingTime,
				simulationSpeedProfile.getCurrentSpeed(),
				generalSimulationConfigData._endSimulationPhysicsTimeInSeconds,
				timeMgr.getCurrentPhysicsElapsedTime()
			);

			if (hasUI)
			{
				_uiFields->pushField(STORM_PROGRESS_REMAINING_TIME_NAME);
			}
			else if ((forcedPushFrameIterator % 32) == 0)
			{
				LOG_DEBUG << STORM_PROGRESS_REMAINING_TIME_NAME << " : " << Storm::toStdString(_progressRemainingTime);
			}
		}

		++forcedPushFrameIterator;

		firstFrame = false;

	} while (true);
}

bool Storm::SimulatorManager::applyCFLIfNeeded(const Storm::GeneralSimulationData &generalSimulationDataConfig)
{
	if (generalSimulationDataConfig._computeCFL)
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
			const float maxDistanceAllowed = generalSimulationDataConfig._particleRadius * 2.f;
			newDeltaTimeStep = computeCFLDistance(generalSimulationDataConfig) / currentStepMaxVelocityNorm;
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

		if (newDeltaTimeStep > generalSimulationDataConfig._maxCFLTime)
		{
			newDeltaTimeStep = generalSimulationDataConfig._maxCFLTime;
		}

		Storm::ITimeManager &timeMgr = Storm::SingletonHolder::instance().getSingleton<Storm::ITimeManager>();

		/* Apply the new timestep */
		return timeMgr.setCurrentPhysicsDeltaTime(newDeltaTimeStep);
	}

	return false;
}

void Storm::SimulatorManager::initializePreSimulation()
{
	const float k_kernelLength = this->getKernelLength();
	for (auto &particleSystem : _particleSystem)
	{
		Storm::ParticleSystem &pSystem = *particleSystem.second;
		pSystem.initializePreSimulation(_particleSystem, k_kernelLength);
	}

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
	const Storm::GeneralSimulationData &generalSimulationConfigData = configMgr.getGeneralSimulationData();

	_sphSolver = Storm::instantiateSPHSolver(Storm::SolverCreationParameter{
		._simulationMode = generalSimulationConfigData._simulationMode,
		._kernelLength = k_kernelLength,
		._particleSystems = &_particleSystem
	});
}

void Storm::SimulatorManager::subIterationStart()
{
	for (auto &particleSystem : _particleSystem)
	{
		particleSystem.second->onSubIterationStart(_particleSystem, _blowers);
	}
}

void Storm::SimulatorManager::revertIteration()
{
	for (auto &particleSystem : _particleSystem)
	{
		particleSystem.second->revertToCurrentTimestep(_blowers);
	}
}

void Storm::SimulatorManager::flushPhysics(const float deltaTime)
{
	Storm::IPhysicsManager &physicsMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IPhysicsManager>();

	for (auto &particleSystem : _particleSystem)
	{
		Storm::ParticleSystem &pSystem = *particleSystem.second;
		// Only non static rigidbody have a changing physics state managed by PhysX engine.
		if (!pSystem.isFluids() && !pSystem.isStatic())
		{
			physicsMgr.applyLocalForces(particleSystem.first, pSystem.getPositions(), pSystem.getForces());
		}
	}

	physicsMgr.update(deltaTime);

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
	for (auto &particleSystem : _particleSystem)
	{
		Storm::ParticleSystem &pSystem = *particleSystem.second;
		pSystem.updatePosition(0.f, true);
	}
}

void Storm::SimulatorManager::refreshParticleNeighborhood()
{
	this->refreshParticlePartition();

	for (auto &particleSystem : _particleSystem)
	{
		Storm::ParticleSystem &pSystem = *particleSystem.second;
		pSystem.buildNeighborhood(_particleSystem);
	}
}

void Storm::SimulatorManager::addFluidParticleSystem(unsigned int id, std::vector<Storm::Vector3> particlePositions)
{
	const std::size_t initialParticleCount = particlePositions.size();
	LOG_COMMENT << "Creating fluid particle system with " << initialParticleCount << " particles.";

	const Storm::GeneralSimulationData &generalConfigData = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>().getGeneralSimulationData();
	if (generalConfigData._removeFluidParticleCollidingWithRb)
	{
		LOG_COMMENT << "Removing fluid particles that collide with rigid bodies particles.";

		removeParticleInsideRbPosition(particlePositions, _particleSystem, generalConfigData._particleRadius);

		LOG_DEBUG << "We removed " << initialParticleCount - particlePositions.size() << " particle(s) after checking which collide with existing rigid bodies.";
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

void Storm::SimulatorManager::loadBlower(const Storm::BlowerData &blowerData)
{
#define STORM_XMACRO_GENERATE_ELEMENTARY_BLOWER(BlowerTypeName, BlowerTypeXmlName, EffectAreaType, MeshMakerType) \
case Storm::BlowerType::BlowerTypeName: appendNewBlower<Storm::BlowerType::BlowerTypeName, Storm::EffectAreaType>(_blowers, blowerData); break;

		switch (blowerData._blowerType)
		{
			STORM_XMACRO_GENERATE_BLOWERS_CODE;

		default:
			Storm::throwException<std::exception>("Unhandled Blower Type creation requested! Value was " + std::to_string(static_cast<int>(blowerData._blowerType)));
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
	if (_raycastEnabled)
	{
		LOG_DEBUG << "Disabling Raycast";
		_raycastEnabled = false;
	}
	else
	{
		LOG_DEBUG << "Enabling Raycast";
		_raycastEnabled = true;
	}
}

float Storm::SimulatorManager::getKernelLength() const
{
	return _kernelHandler.getKernelValue();
}

void Storm::SimulatorManager::pushParticlesToGraphicModule(bool ignoreDirty, bool pushParallel /*= true*/) const
{
	Storm::IGraphicsManager &graphicMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IGraphicsManager>();

	if (_particleSelector.hasSelectedParticle())
	{
		if (auto found = _particleSystem.find(_particleSelector.getSelectedParticleSystemId()); found != std::end(_particleSystem))
		{
			const Storm::ParticleSystem &selectedParticleSystem = *found->second;

			const std::size_t selectedParticleIndex = _particleSelector.getSelectedParticleIndex();

			const Storm::Vector3 &selectedParticlePosition = selectedParticleSystem.getPositions()[selectedParticleIndex];
			graphicMgr.pushParticleSelectionForceData(_particleSelector.getSelectedForcePosition(selectedParticlePosition), _particleSelector.getSelectedForceToDisplay());
		}
	}
	
	const auto pushActionLambda = [&graphicMgr, ignoreDirty](const auto &particleSystemPair)
	{
		const Storm::ParticleSystem &currentParticleSystem = *particleSystemPair.second;
		if (ignoreDirty || currentParticleSystem.isDirty() || currentParticleSystem.isFluids())
		{
			graphicMgr.pushParticlesData(particleSystemPair.first, currentParticleSystem.getPositions(), currentParticleSystem.getVelocity(), currentParticleSystem.isFluids(), currentParticleSystem.isWall());
		}
	};

	if (pushParallel)
	{
		Storm::runParallel(_particleSystem, pushActionLambda);
	}
	else
	{
		for (const auto &particleSystPtr : _particleSystem)
		{
			pushActionLambda(particleSystPtr);
		}
	}
}

void Storm::SimulatorManager::cycleSelectedParticleDisplayMode()
{
	_particleSelector.cycleParticleSelectionDisplayMode();

	if (_particleSelector.hasSelectedParticle())
	{
		const Storm::ITimeManager &timeMgr = Storm::SingletonHolder::instance().getSingleton<Storm::ITimeManager>();
		if (timeMgr.getStateNoSyncWait() == Storm::TimeWaitResult::Pause)
		{
			this->pushParticlesToGraphicModule(true, false);
		}
	}
}

void Storm::SimulatorManager::refreshParticleSelection()
{
	if (_particleSelector.hasSelectedParticle())
	{
		if (auto found = _particleSystem.find(_particleSelector.getSelectedParticleSystemId()); found != std::end(_particleSystem))
		{
			const Storm::ParticleSystem &pSystem = *found->second;

			const std::size_t selectedParticleIndex = _particleSelector.getSelectedParticleIndex();
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

void Storm::SimulatorManager::exitWithCode(Storm::ExitCode code)
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
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

	const Storm::RecordConfigData &recordConfig = singletonHolder.getSingleton<Storm::IConfigManager>().getRecordConfigData();

	Storm::SerializeRecordHeader recordHeader;
	recordHeader._recordFrameRate = recordConfig._recordFps;

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
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::RecordConfigData &recordConfig = singletonHolder.getSingleton<Storm::IConfigManager>().getRecordConfigData();

	Storm::SerializeRecordPendingData currentFrameData;
	currentFrameData._physicsTime = currentPhysicsTime;

	for (const auto &particleSystemPair : _particleSystem)
	{
		const Storm::ParticleSystem &pSystemRef = *particleSystemPair.second;

		if (pushStatics || !pSystemRef.isStatic())
		{
			Storm::SerializeRecordParticleSystemData &framePSystemElementData = currentFrameData._particleSystemElements.emplace_back();

			framePSystemElementData._systemId = particleSystemPair.first;

			if (!pSystemRef.isFluids())
			{
				const Storm::RigidBodyParticleSystem &pSystemRefAsRb = static_cast<const Storm::RigidBodyParticleSystem &>(pSystemRef);
				framePSystemElementData._pSystemPosition = pSystemRefAsRb.getRbPosition();
				framePSystemElementData._pSystemGlobalForce = pSystemRefAsRb.getRbTotalForce();
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

	this->pushParticlesToGraphicModule(true, pushParallel);

	Storm::IPhysicsManager &physicsMgr = singletonHolder.getSingleton<Storm::IPhysicsManager>();
	physicsMgr.pushConstraintsRecordedFrame(frame._constraintElements);
}

void Storm::SimulatorManager::resetReplay()
{
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
				const Storm::RecordConfigData &recordConfig = configMgr.getRecordConfigData();
				this->applyReplayFrame(frameBefore, recordConfig._recordFps, false);
				
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
		Storm::throwException<std::exception>("Particle system with id " + std::to_string(id) + " is unknown!");
	}
}

void Storm::SimulatorManager::printFluidParticleData() const
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
}

const Storm::ParticleSystem& Storm::SimulatorManager::getParticleSystem(unsigned int id) const
{
	if (const auto foundParticleSystem = _particleSystem.find(id); foundParticleSystem != std::end(_particleSystem))
	{
		return *foundParticleSystem->second;
	}
	else
	{
		Storm::throwException<std::exception>("Particle system with id " + std::to_string(id) + " is unknown!");
	}
}

void Storm::SimulatorManager::executeAllForcesCheck()
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	Storm::IPhysicsManager &physicsMgr = singletonHolder.getSingleton<Storm::IPhysicsManager>();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

	const Storm::GeneralSimulationData &generalConfig = configMgr.getGeneralSimulationData();
	const Storm::Vector3 &gravityAccel = generalConfig._gravity;

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
