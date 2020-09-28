﻿#include "SimulatorManager.h"

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
#include "TimeWaitResult.h"

#include "ParticleSystem.h"
#include "FluidParticleSystem.h"
#include "RigidBodyParticleSystem.h"

#include "GeneralSimulationData.h"
#include "FluidData.h"

#include "SimulationMode.h"
#include "KernelMode.h"

#include "PartitionSelection.h"

#include "SemiImplicitEulerSolver.h"
#include "Kernel.h"

#include "SPHSolvers.h"

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

#include "RaycastQueryRequest.h"
#include "RaycastHitResult.h"

#include <fstream>


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
		static std::string parse(const Storm::ParticleNeighborhoodArray &neighborhood)
		{
			std::string result;

			const std::size_t neighborhoodCount = neighborhood.size();
			result.reserve(12 + neighborhoodCount * 70);

			result += "Count :";
			result += std::to_string(neighborhoodCount);
			
			for (const Storm::NeighborParticleInfo &neighbor : neighborhood)
			{
				result += "; iter=";
				result += Storm::toStdString<PolicyType>(neighbor._particleIndex);
				result += "; r=";
				result += Storm::toStdString<PolicyType>(neighbor._vectToParticleNorm);
				result += "; xij=";
				result += Storm::toStdString<PolicyType>(neighbor._positionDifferenceVector);
				result += "; fluid=";
				result += Storm::toStdString<PolicyType>(neighbor._isFluidParticle);
			}

			return result;
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
			_profileMgrPtr{ profileMgrPtr }
		{
			if (profileMgrPtr)
			{
				_profileMgrPtr->startSpeedProfile(k_simulationSpeedBalistName);
			}
		}

		~SpeedProfileBalist()
		{
			if (_profileMgrPtr)
			{
				_profileMgrPtr->endSpeedProfile(k_simulationSpeedBalistName);
			}
		}

	private:
		Storm::IProfilerManager*const _profileMgrPtr;
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

	void removeParticleInsideRbPosition(std::vector<Storm::Vector3> &inOutParticlePositions, const std::map<unsigned int, std::unique_ptr<Storm::ParticleSystem>> &allParticleSystem, const float particleRadius)
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
}

Storm::SimulatorManager::SimulatorManager() :
	_raycastEnabled{ false }
{

}

Storm::SimulatorManager::~SimulatorManager() = default;

void Storm::SimulatorManager::initialize_Implementation()
{
	LOG_COMMENT << "Initializing the simulator";

	/* Initialize kernels */

	Storm::initializeKernels(this->getKernelLength());

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

	Storm::IInputManager &inputMgr = singletonHolder.getSingleton<Storm::IInputManager>();
	inputMgr.bindKey(Storm::SpecialKey::KC_F1, [this]() { this->printFluidParticleData(); });
	inputMgr.bindKey(Storm::SpecialKey::KC_E, [this]() { this->tweekBlowerEnabling(); });
	inputMgr.bindKey(Storm::SpecialKey::KC_R, [this]() { this->tweekRaycastEnabling(); });

	Storm::IRaycastManager &raycastMgr = singletonHolder.getSingleton<Storm::IRaycastManager>();
	inputMgr.bindMouseLeftClick([this, &raycastMgr, &singletonHolder](int xPos, int yPos, int width, int height)
	{
		if (_raycastEnabled)
		{
			raycastMgr.queryRaycast(Storm::Vector2{ xPos, yPos }, std::move(Storm::RaycastQueryRequest{ [this, &singletonHolder](std::vector<Storm::RaycastHitResult> &&result)
			{
				bool hasMadeSelectionChanges;

				Storm::IGraphicsManager &graphicMgr = singletonHolder.getSingleton<Storm::IGraphicsManager>();
				if (result.empty())
				{
					hasMadeSelectionChanges = _particleSelector.clearParticleSelection();
					LOG_DEBUG << "No particle touched";
				}
				else
				{
					const Storm::RaycastHitResult &firstHit = result[0];

					hasMadeSelectionChanges = _particleSelector.setParticleSelection(firstHit._systemId, firstHit._particleId);

					LOG_DEBUG <<
						"Raycast touched particle " << firstHit._particleId << " inside system id " << firstHit._systemId << "\n"
						"Hit Position : " << firstHit._hitPosition;
				}

				if (hasMadeSelectionChanges && singletonHolder.getSingleton<Storm::ITimeManager>().getStateNoSyncWait() == Storm::TimeWaitResult::Pause)
				{
					this->pushParticlesToGraphicModule(true, false);
				}
			} }.addPartitionFlag(Storm::PartitionSelection::DynamicRigidBody)
			   .firstHitOnly())
			);
		}
	});

	/* Register this thread as the simulator thread for the speed profiler */
	Storm::IProfilerManager &profilerMgr = singletonHolder.getSingleton<Storm::IProfilerManager>();
	profilerMgr.registerCurrentThreadAsSimulationThread(k_simulationSpeedBalistName);
}

void Storm::SimulatorManager::cleanUp_Implementation()
{
	// TODO
}

void Storm::SimulatorManager::run()
{
	LOG_COMMENT << "Starting simulation loop";

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

	Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();
	Storm::IPhysicsManager &physicsMgr = singletonHolder.getSingleton<Storm::IPhysicsManager>();
	Storm::IThreadManager &threadMgr = singletonHolder.getSingleton<Storm::IThreadManager>();
	Storm::ISpacePartitionerManager &spacePartitionerMgr = singletonHolder.getSingleton<Storm::ISpacePartitionerManager>();

	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
	const Storm::GeneralSimulationData &generalSimulationConfigData = configMgr.getGeneralSimulationData();
	
	Storm::IProfilerManager* profilerMgrNullablePtr = configMgr.getShouldProfileSimulationSpeed() ? singletonHolder.getFacet<Storm::IProfilerManager>() : nullptr;
	
	std::vector<Storm::SimulationCallback> tmpSimulationCallback;
	tmpSimulationCallback.reserve(8);

	this->pushParticlesToGraphicModule(true);
	
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

	this->initializePreSimulation();

	// A fast iterator that loops every 256 iterations.
	unsigned char forcedPushFrameIterator = 0;

	bool firstFrame = true;

	do
	{
		Storm::TimeWaitResult simulationState = generalSimulationConfigData._simulationNoWait ? timeMgr.getStateNoSyncWait() : timeMgr.waitNextFrame();
		switch (simulationState)
		{
		case Storm::TimeWaitResult::Exit:
			return;

		case TimeWaitResult::Pause:
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

		SpeedProfileBalist simulationSpeedProfile{ profilerMgrNullablePtr };

		this->executeIteration(firstFrame, forcedPushFrameIterator);

		// Push all particle data to the graphic module to be rendered...
		this->pushParticlesToGraphicModule(forcedPushFrameIterator == 0);

		// Takes time to process messages that came from other threads.
		threadMgr.processCurrentThreadActions();

		timeMgr.advanceCurrentPhysicsElapsedTime();

		++forcedPushFrameIterator;
		firstFrame = false;

	} while (true);
}

void Storm::SimulatorManager::executeIteration(bool firstFrame, unsigned char forcedPushFrameIterator)
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

	Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();
	Storm::IPhysicsManager &physicsMgr = singletonHolder.getSingleton<Storm::IPhysicsManager>();
	Storm::IThreadManager &threadMgr = singletonHolder.getSingleton<Storm::IThreadManager>();
	Storm::ISpacePartitionerManager &spacePartitionerMgr = singletonHolder.getSingleton<Storm::ISpacePartitionerManager>();

	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
	const Storm::GeneralSimulationData &generalSimulationConfigData = configMgr.getGeneralSimulationData();

	float physicsElapsedDeltaTime = timeMgr.getCurrentPhysicsDeltaTime();
	const float physicsCurrentTime = timeMgr.getCurrentPhysicsElapsedTime();

	// initialize for current iteration. I.e. Initializing with gravity and resetting current iteration velocity.
	// Also build neighborhood.

	if (!firstFrame && forcedPushFrameIterator % generalSimulationConfigData._recomputeNeighborhoodStep == 0)
	{
		spacePartitionerMgr.clearSpaceReorderingNoStatic();
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

	for (const std::unique_ptr<Storm::IBlower> &blowerUPtr : _blowers)
	{
		blowerUPtr->advanceTime(physicsElapsedDeltaTime);
	}

	for (auto &particleSystem : _particleSystem)
	{
		particleSystem.second->initializeIteration(_particleSystem, _blowers);
	}

	bool runIterationAgain;

	int maxCFLIteration = generalSimulationConfigData._maxCFLIteration;
	int iter = 0;

	float exDeltaTime = physicsElapsedDeltaTime;
	
	const float kernelLength = this->getKernelLength();

	bool hasRunIterationBefore = false;
	do 
	{
		if (hasRunIterationBefore)
		{
			for (auto &particleSystem : _particleSystem)
			{
				particleSystem.second->revertToCurrentTimestep(_blowers);
			}
		}
		else
		{
			hasRunIterationBefore = true;
		}

		// Compute the simulation
		switch (generalSimulationConfigData._simulationMode)
		{
		case Storm::SimulationMode::WCSPH:
			Storm::WCSPHSolver::execute(_particleSystem, kernelLength);
			break;

		case Storm::SimulationMode::PCISPH:
			Storm::PCISPHSolver::execute(_particleSystem, kernelLength);
			break;

		default:
			Storm::throwException<std::exception>("Unknown simulation mode!");
		}

		float velocityThresholdSquaredForCFL = computeCFLDistance(generalSimulationConfigData) / physicsElapsedDeltaTime;
		velocityThresholdSquaredForCFL = velocityThresholdSquaredForCFL * velocityThresholdSquaredForCFL;

		bool shouldApplyCFL = false;

		// Semi implicit Euler to update the particle position
		for (auto &particleSystem : _particleSystem)
		{
			shouldApplyCFL |= particleSystem.second->computeVelocityChange(physicsElapsedDeltaTime, velocityThresholdSquaredForCFL);
		}

		runIterationAgain = this->applyCFLIfNeeded(generalSimulationConfigData);
		if (runIterationAgain)
		{
			physicsElapsedDeltaTime = timeMgr.getCurrentPhysicsDeltaTime();
			
			if (physicsElapsedDeltaTime != exDeltaTime)
			{
				// Correct the blower time to the corrected time
				float correctionDeltaTime = physicsElapsedDeltaTime - exDeltaTime;
				exDeltaTime = physicsElapsedDeltaTime;
				for (const std::unique_ptr<Storm::IBlower> &blowerUPtr : _blowers)
				{
					blowerUPtr->advanceTime(correctionDeltaTime);
				}
			}
		}

		++iter;

	} while (runIterationAgain && iter < maxCFLIteration && physicsElapsedDeltaTime < generalSimulationConfigData._maxCFLTime && physicsElapsedDeltaTime > getMinCLFTime());

	// Update everything that should be updated once CFL iteration finished. 
	for (auto &particleSystem : _particleSystem)
	{
		particleSystem.second->postApplySPH(physicsElapsedDeltaTime);
	}

	// Update the Rigid bodies positions in scene
	physicsMgr.update(physicsElapsedDeltaTime);

	// Update the position of every particles. 
	for (auto &particleSystem : _particleSystem)
	{
		particleSystem.second->updatePosition(physicsElapsedDeltaTime, false);
	}
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
}

void Storm::SimulatorManager::refreshParticlesPosition()
{
	for (auto &particleSystem : _particleSystem)
	{
		Storm::ParticleSystem &pSystem = *particleSystem.second;
		pSystem.updatePosition(0.f, true);
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
	const Storm::GeneralSimulationData &generalSimulData = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>().getGeneralSimulationData();
	return generalSimulData._particleRadius * generalSimulData._kernelCoefficient;
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
			graphicMgr.pushParticleSelectionForceData(selectedParticleSystem.getPositions()[selectedParticleIndex], _particleSelector.getSelectedParticleForceToDisplay());
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
