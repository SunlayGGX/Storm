#include "SimulatorManager.h"

#include "SingletonHolder.h"
#include "IPhysicsManager.h"
#include "IThreadManager.h"
#include "IGraphicsManager.h"
#include "IConfigManager.h"
#include "ITimeManager.h"
#include "TimeWaitResult.h"

#include "ParticleSystem.h"
#include "FluidParticleSystem.h"
#include "RigidBodyParticleSystem.h"

#include "GeneralSimulationData.h"
#include "FluidData.h"

#include "SimulationMode.h"
#include "KernelMode.h"

#include "DensitySolver.h"
#include "SemiImplicitEulerSolver.h"
#include "SpikyKernel.h"
#include "CubicSplineKernel.h"


namespace
{
	template<class ParticleSystemType, class MapType, class ... Args>
	void addParticleSystemToMap(MapType &map, unsigned int particleSystemId, Args &&... args)
	{
		std::unique_ptr<Storm::ParticleSystem> particleSystemPtr = std::make_unique<ParticleSystemType>(particleSystemId, std::forward<Args>(args)...);
		map[particleSystemId] = std::move(particleSystemPtr);
	}

	std::function<float(float)> retrieveGradKernelMethod(const Storm::KernelMode kernelMode, const float kernelLength)
	{
		switch (kernelMode)
		{
		case Storm::KernelMode::Muller2013: return Storm::GradientSpikyKernel{ kernelLength };
		case Storm::KernelMode::CubicSpline: return Storm::GradientCubicSplineKernel{ kernelLength };
		}

		Storm::throwException<std::exception>("Unknown kernel mode!");
	}
}


Storm::SimulatorManager::SimulatorManager() = default;
Storm::SimulatorManager::~SimulatorManager() = default;

void Storm::SimulatorManager::initialize_Implementation()
{
	/* Initialize _kernelScale : This is the Shepard filter scale that should be initialized for when a particle is considered having its full neighborhood */

	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const Storm::GeneralSimulationData &generalSimulConfig = configMgr.getGeneralSimulationData();
	
	const float k_kernelLength = this->getKernelLength();

	const auto gradViscoPressureKernel = retrieveGradKernelMethod(generalSimulConfig._kernelMode, k_kernelLength);

	float sumKernel = 0.f;

	// Simulate a full neighborhood around a particle (positions are relative) when it is stable (radius is exactly the kernel length).
	for (int xNeighborIndex = -1; xNeighborIndex <= 1; ++xNeighborIndex)
	{
		for (int yNeighborIndex = -1; yNeighborIndex <= 1; ++yNeighborIndex)
		{
			for (int zNeighborIndex = -1; zNeighborIndex <= 1; ++zNeighborIndex)
			{
				// If x, y and z are 0, then the particle position index are the currentP...
				// But currentP is not part of its own neighborhood so we should ignore it.
				if (!(xNeighborIndex == 0 && yNeighborIndex == 0 && zNeighborIndex == 0))
				{
					Storm::Vector3 relativeNeighborPositionToCurrentP{
						xNeighborIndex * k_kernelLength,
						yNeighborIndex * k_kernelLength,
						zNeighborIndex * k_kernelLength
					};

					const float radiusNeighborToCurrentP = relativeNeighborPositionToCurrentP.norm();

					// ??? FIXME
					sumKernel += gradViscoPressureKernel(radiusNeighborToCurrentP);
				}
			}
		}
	}

	// Initialize the kernel scale for each particle system.
	for (auto &particleSystemPair : _particleSystem)
	{
		Storm::ParticleSystem &currentParticleSystem = *particleSystemPair.second;
		currentParticleSystem.setKernelScale(currentParticleSystem.getRestDensity() / (currentParticleSystem.getMassPerParticle() * sumKernel));
	}
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
	const Storm::GeneralSimulationData &generalSimulationConfigData = singletonHolder.getSingleton<Storm::IConfigManager>().getGeneralSimulationData();
	
	std::vector<Storm::SimulationCallback> tmpSimulationCallback;
	tmpSimulationCallback.reserve(8);

	// A fast iterator that loops every 1024 iterations.
	union { unsigned short _val : 10 = 0; } _forcedPushFrameIterator;

	do
	{
		Storm::TimeWaitResult simulationState = timeMgr.waitNextFrame();
		switch (simulationState)
		{
		case Storm::TimeWaitResult::Exit:
			return;

		case TimeWaitResult::Pause:
			// Takes time to process messages that came from other threads.
			threadMgr.processCurrentThreadActions();
			continue;

		case TimeWaitResult::Continue:
		default:
			break;
		}

		const float physicsElapsedDeltaTime = timeMgr.getCurrentPhysicsDeltaTime();

		// initialize for current iteration. I.e. Initializing with gravity and resetting current iteration velocity.
		for (auto &particleSystem : _particleSystem)
		{
			particleSystem.second->initializeIteration();
		}

		// Build neighborhood.
		// TODO: Optimize
		for (auto &particleSystem : _particleSystem)
		{
			particleSystem.second->buildNeighborhood(_particleSystem);
		}

		// Compute the simulation
		switch (generalSimulationConfigData._simulationMode)
		{
		case Storm::SimulationMode::PCISPH:
			this->executePCISPH(generalSimulationConfigData, physicsElapsedDeltaTime);
			break;
		}

		for (auto &particleSystem : _particleSystem)
		{
			particleSystem.second->postApplySPH();
		}

		// Update the Rigid bodies positions in scene
		physicsMgr.update(physicsElapsedDeltaTime);


		// Semi implicit Euler to update the particle position
		for (auto &particleSystem : _particleSystem)
		{
			particleSystem.second->updatePosition(physicsElapsedDeltaTime);
		}

		// Apply CFL (???)
		this->applyCFLIfNeeded(generalSimulationConfigData);

		// Push all particle data to the graphic module to be rendered...
		// The first 5 frames every 1024 frames will be pushed for sure to the graphic module to be sure everyone is sync... 
		this->pushParticlesToGraphicModule(_forcedPushFrameIterator._val < 5);


		// Takes time to process messages that came from other threads.
		threadMgr.processCurrentThreadActions();

		++_forcedPushFrameIterator._val;

	} while (true);
}

void Storm::SimulatorManager::executePCISPH(const Storm::GeneralSimulationData &generalSimulationDataConfig, float physicsElapsedDeltaTime)
{
	const float k_maxDensityError = generalSimulationDataConfig._maxDensityError;
	const float k_invertPhysicsTimeSquared = 1.f / (physicsElapsedDeltaTime * physicsElapsedDeltaTime);

	const float k_currentKernelLength = this->getKernelLength();

	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const Storm::FluidData &fluidConfData = configMgr.getFluidData();

	const auto gradViscoPressureKernel = retrieveGradKernelMethod(generalSimulationDataConfig._kernelMode, k_currentKernelLength);

	for (auto &particleSystemPair : _particleSystem)
	{
		Storm::ParticleSystem &particleSystem = *particleSystemPair.second;

		if (particleSystem.isFluids())
		{
			std::vector<float> &densities = particleSystem.getDensities();
			std::vector<float> &pressures = particleSystem.getPressures();
			std::vector<Storm::Vector3> &forces = particleSystem.getForces();
			std::vector<std::vector<Storm::NeighborParticleInfo>> &neighborhoodArrays = particleSystem.getNeighborhoodArrays();
			std::vector<Storm::Vector3> &velocities = particleSystem.getVelocity();
			const float k_massPerParticle = particleSystem.getMassPerParticle();
			const float k_particleSystemRestDensity = particleSystem.getRestDensity();
			const Storm::Vector3 gravityForce = k_massPerParticle * generalSimulationDataConfig._gravity;

			// Initialization for external forces
			std::for_each(std::execution::par, std::begin(forces), std::end(forces), [&](Storm::Vector3 &force)
			{
				// External force
				const std::size_t currentPIndex = Storm::ParticleSystem::getParticleIndex(forces, force);
				force += gravityForce;

				// TODO : 
				// Blowers

				// Density
				float &currentPDensity = densities[currentPIndex];
				currentPDensity = Storm::DensitySolver::computeDensityPCISPH(k_massPerParticle, neighborhoodArrays[currentPIndex]);

				// Pressure init

				// max is explained inside SplishSplash tutorial : https://interactivecomputergraphics.github.io/SPH-Tutorial/pdf/SPH_Tutorial.pdf
				// Quoted : "As ρi < ρ0 is not considered to solve the particle deficiency problem at the free surface, the computed pressure is always non-negative."
				pressures[currentPIndex] = std::max(fluidConfData._kPressureCoeff * (currentPDensity - k_particleSystemRestDensity), 0.f);

				//pressures[currentPIndex] = 20000.f * (currentPDensity - k_particleSystemRestDensity);
				//pressures[currentPIndex] = currentPDensity - k_particleSystemRestDensity;
			});

			const float k_artificialViscoEpsilonCoeff = 0.01f * k_currentKernelLength * k_currentKernelLength;
			const float k_precomputedViscosityCoeff = k_massPerParticle * 2.f * k_currentKernelLength * fluidConfData._dynamicViscosity * fluidConfData._soundSpeed;

			// Viscosity
			std::for_each(std::execution::par, std::begin(forces), std::end(forces), [&](Storm::Vector3 &force)
			{
				const std::size_t currentPIndex = Storm::ParticleSystem::getParticleIndex(forces, force);
				
				const float currentPDensity = densities[currentPIndex];
				const Storm::Vector3 &currentPVelocity = velocities[currentPIndex];
				std::vector<Storm::NeighborParticleInfo> &currentPNeighborhoodArray = neighborhoodArrays[currentPIndex];

				const float k_currentParticleSystemKernelCoeff = particleSystem.getKernelScale();

				for (const Storm::NeighborParticleInfo &neighborInfo : currentPNeighborhoodArray)
				{
					Storm::ParticleSystem &neighborhoodContainingPSystem = *neighborInfo._containingParticleSystem;

					const float massPerParticle = neighborhoodContainingPSystem.getMassPerParticle();
					const float neighborDensity = neighborhoodContainingPSystem.getDensities()[neighborInfo._particleIndex];
					const Storm::Vector3 &neighborVelocity = neighborhoodContainingPSystem.getVelocity()[neighborInfo._particleIndex];

					const Storm::Vector3 vij = currentPVelocity - neighborVelocity;

					const float viscosityKernelValue = k_currentParticleSystemKernelCoeff * gradViscoPressureKernel(neighborInfo._vectToParticleNorm);

					const float coeff = massPerParticle * k_precomputedViscosityCoeff / ((currentPDensity + neighborDensity) * (neighborInfo._vectToParticleSquaredNorm + k_artificialViscoEpsilonCoeff)) * viscosityKernelValue;

					//const float completeArtificialViscosityCoefficient = coeff * std::min(vij.dot(neighborInfo._positionDifferenceVector), 0.f);
					const float completeArtificialViscosityCoefficient = coeff * vij.dot(neighborInfo._positionDifferenceVector);

					Storm::Vector3 currentViscosityForce = completeArtificialViscosityCoefficient * vij.normalized();

					force += currentViscosityForce;

					if (!neighborInfo._isFluidParticle && !neighborhoodContainingPSystem.isStatic())
					{
						Storm::Vector3 &rbParticleForce = neighborhoodContainingPSystem.getForces()[neighborInfo._particleIndex];

						std::lock_guard<std::mutex> lock{ neighborInfo._containingParticleSystem->_mutex };
						// F a->b = -F b->a
						rbParticleForce -= currentViscosityForce;
					}
				}
			});
		}
	}

	// Pressure prediction and solving
	unsigned int currentPredictionIteration = 0;
	bool runPrediction = true;
	while (runPrediction && currentPredictionIteration < generalSimulationDataConfig._maxPredictIteration)
	{
		// Note "currentP" in variable name = currentParticle (was just lazy to write and read "Particle" each time.

		// First : predict for all particle the projected predicted positions.
		for (auto &particleSystemPair : _particleSystem)
		{
			Storm::ParticleSystem &particleSystem = *particleSystemPair.second;

			if (particleSystem.isFluids())
			{
				const std::vector<Storm::Vector3> &positions = particleSystem.getPositions();
				const std::vector<Storm::Vector3> &velocities = particleSystem.getVelocity();
				const std::vector<Storm::Vector3> &forces = particleSystem.getForces();
				const std::vector<Storm::Vector3> &pressureForces = particleSystem.getPredictedPressureForces();
				std::vector<Storm::Vector3> &predictedPositions = particleSystem.getPredictedPositions();
				const float k_massPerParticle = particleSystem.getMassPerParticle();

				std::for_each(std::execution::par, std::begin(predictedPositions), std::end(predictedPositions), [&](Storm::Vector3 &currentPPredictedPosition)
				{
					const std::size_t currentPIndex = Storm::ParticleSystem::getParticleIndex(predictedPositions, currentPPredictedPosition);

					const Storm::Vector3 &currentPVelocity = velocities[currentPIndex];
					const Storm::Vector3 &currentPForce = forces[currentPIndex];
					const Storm::Vector3 &currentPPredictPressureForce = pressureForces[currentPIndex];

					Storm::SemiImplicitEulerSolver solver{ k_massPerParticle, currentPForce + currentPPredictPressureForce, currentPVelocity, physicsElapsedDeltaTime };

					const Storm::Vector3 &currentPPosition = positions[currentPIndex];
					currentPPredictedPosition = currentPPosition + solver._positionDisplacment;
				});
			}

			// Second : Compute the predicted pressure scalar field
			for (auto &particleSystemPair : _particleSystem)
			{
				Storm::ParticleSystem &particleSystem = *particleSystemPair.second;

				if (particleSystem.isFluids())
				{
					const std::vector<std::vector<Storm::NeighborParticleInfo>> &neighborhoodArrays = particleSystem.getNeighborhoodArrays();
					const std::vector<Storm::Vector3> &predictedPositions = particleSystem.getPredictedPositions();
					std::vector<float> &pressures = particleSystem.getPressures();
					std::vector<float> &predictedDensities = particleSystem.getPredictedDensities();
					const float k_massPerParticle = particleSystem.getMassPerParticle();
					const float k_particleSystemRestDensity = particleSystem.getRestDensity();

					std::atomic<bool> runPredictionTmp = false;

					std::for_each(std::execution::par, std::begin(pressures), std::end(pressures), [&](float &currentPPredictedPressure)
					{
						const std::size_t currentPIndex = Storm::ParticleSystem::getParticleIndex(pressures, currentPPredictedPressure);

						const Storm::Vector3 &currentPPredictedPosition = predictedPositions[currentPIndex];
						const std::vector<Storm::NeighborParticleInfo> &currentPNeighbors = neighborhoodArrays[currentPIndex];

						float kCoeff = 0.f;
						float kernelGradSum = 0.f;

						const float k_currentParticleSystemKernelCoeff = particleSystem.getKernelScale();

						for (const Storm::NeighborParticleInfo &pNeighbor : currentPNeighbors)
						{
							if (pNeighbor._isFluidParticle)
							{
								const Storm::Vector3 &predictedNeighborPPosition = pNeighbor._containingParticleSystem->getPredictedPositions()[pNeighbor._particleIndex];
								Storm::Vector3 diffPositionVectorXij = currentPPredictedPosition - predictedNeighborPPosition;
								kernelGradSum += k_currentParticleSystemKernelCoeff * gradViscoPressureKernel(diffPositionVectorXij.norm());
							}
							else
							{
								const float neighborDensity = pNeighbor._containingParticleSystem->getDensities()[pNeighbor._particleIndex];
								kCoeff += k_particleSystemRestDensity * neighborDensity * k_currentParticleSystemKernelCoeff * gradViscoPressureKernel(pNeighbor._vectToParticleNorm);
							}
						}

						float &currentPPredictedDensity = predictedDensities[currentPIndex];
						currentPPredictedDensity = (k_massPerParticle * kernelGradSum) + kCoeff;

						const float currentPDensityError = currentPPredictedDensity - k_particleSystemRestDensity;
						runPredictionTmp = std::fabs(currentPDensityError) > k_maxDensityError || runPredictionTmp;

						currentPPredictedPressure += currentPDensityError * k_invertPhysicsTimeSquared;
					});

					runPrediction = runPredictionTmp;
				}
			}

			// Third (added) : rescale the pressure to not have any negative one
			float totalMinPressure = 0.f;
			for (auto &particleSystemPair : _particleSystem)
			{
				Storm::ParticleSystem &particleSystem = *particleSystemPair.second;

				if (particleSystem.isFluids())
				{
					std::vector<float> &pressures = particleSystem.getPressures();
					const float minPressure = *std::min_element(std::execution::par, std::begin(pressures), std::end(pressures));
					if (minPressure < totalMinPressure)
					{
						totalMinPressure = minPressure;
					}
				}
			}
			if (totalMinPressure < 0.f)
			{
				for (auto &particleSystemPair : _particleSystem)
				{
					Storm::ParticleSystem &particleSystem = *particleSystemPair.second;

					if (particleSystem.isFluids())
					{
						std::vector<float> &pressures = particleSystem.getPressures();
						std::for_each(std::execution::par, std::begin(pressures), std::end(pressures), [pressureOffset = -totalMinPressure](float &currentPPredictedPressure)
						{
							currentPPredictedPressure += pressureOffset;
						});
					}
				}
			}

			// Fourth : Compute the predicted pressure force
			for (auto &particleSystemPair : _particleSystem)
			{
				Storm::ParticleSystem &particleSystem = *particleSystemPair.second;

				if (particleSystem.isFluids())
				{
					const std::vector<std::vector<Storm::NeighborParticleInfo>> &neighborhoodArrays = particleSystem.getNeighborhoodArrays();
					std::vector<Storm::Vector3> &pressureForces = particleSystem.getPredictedPressureForces();
					std::vector<float> &predictedDensities = particleSystem.getPredictedDensities();
					std::vector<float> &pressures = particleSystem.getPressures();
					const float k_massPerParticle = particleSystem.getMassPerParticle();
					const float k_fluidMassCoeff = -k_massPerParticle * k_massPerParticle;
					const float k_rbMassCoeff = -k_massPerParticle * particleSystem.getRestDensity();
					std::for_each(std::execution::par, std::begin(pressureForces), std::end(pressureForces), [&](Storm::Vector3 &currentPPredictedPressure)
					{
						const std::size_t currentPIndex = Storm::ParticleSystem::getParticleIndex(pressureForces, currentPPredictedPressure);
						const float currentPPredictedDensity = predictedDensities[currentPIndex];
						const float currentPPressure = pressures[currentPIndex];
						const float currentPFluidPressureDensityCoeff = currentPPressure / (currentPPredictedDensity * currentPPredictedDensity);
						const std::vector<Storm::NeighborParticleInfo> &currentPNeighbors = neighborhoodArrays[currentPIndex];

						const float k_currentParticleSystemKernelCoeff = particleSystem.getKernelScale();

						for (const Storm::NeighborParticleInfo &pNeighbor : currentPNeighbors)
						{
							const float gradientCoeff = k_currentParticleSystemKernelCoeff * gradViscoPressureKernel(pNeighbor._vectToParticleNorm);
							const float neighborDensity = pNeighbor._containingParticleSystem->getDensities()[pNeighbor._particleIndex];

							float coeff;

							if (pNeighbor._isFluidParticle)
							{
								const float neighborPressure = pNeighbor._containingParticleSystem->getPressures()[pNeighbor._particleIndex];
								coeff = k_fluidMassCoeff * gradientCoeff * (currentPFluidPressureDensityCoeff + (neighborPressure / (neighborDensity * neighborDensity)));
							}
							else
							{
								const float rbCoeff = k_rbMassCoeff * neighborDensity;
								coeff = rbCoeff * gradientCoeff * currentPFluidPressureDensityCoeff;
							}

							const Storm::Vector3 currentPressureForceAdded = (coeff / pNeighbor._vectToParticleNorm) * pNeighbor._positionDifferenceVector;
							currentPPredictedPressure += currentPressureForceAdded;

							if (!runPrediction && !pNeighbor._isFluidParticle && !pNeighbor._containingParticleSystem->isStatic())
							{
								Storm::Vector3 &rbParticleForce = pNeighbor._containingParticleSystem->getForces()[pNeighbor._particleIndex];

								std::lock_guard<std::mutex> lock{ pNeighbor._containingParticleSystem->_mutex };
								// F a->b = -F b->a
								rbParticleForce -= currentPressureForceAdded;
							}
						}
					});
				}
			}

			++currentPredictionIteration;
		}
	}

	// Finally The predicted pressure (which isn't predicted anymore) should be added to the total force applied to the particle
	for (auto &particleSystemPair : _particleSystem)
	{
		particleSystemPair.second->applyPredictedPressureToTotalForce();
	}
}

void Storm::SimulatorManager::applyCFLIfNeeded(const Storm::GeneralSimulationData &generalSimulationDataConfig)
{
	float newDeltaTimeStep = generalSimulationDataConfig._physicsTimeInSeconds;
	if (newDeltaTimeStep <= 0.f)
	{
		// 500ms by default seems fine (this time will be the one set if no particle moves)...
		newDeltaTimeStep = 0.500f;

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
			newDeltaTimeStep = generalSimulationDataConfig._kernelCoefficient * maxDistanceAllowed / currentStepMaxVelocityNorm;
		}
		else if (std::isinf(currentStepMaxVelocityNorm) || std::isnan(currentStepMaxVelocityNorm))
		{
			LOG_WARNING << "Simulation had exploded (at least one particle had an infinite or NaN velocity)!";
		}

		// The physics engine doesn't like when the timestep is below some value...
		constexpr float minDeltaTime = 0.0001f;
		if (newDeltaTimeStep < minDeltaTime)
		{
			newDeltaTimeStep = minDeltaTime;
		}

		/* Apply the new timestep */
		Storm::SingletonHolder::instance().getSingleton<Storm::ITimeManager>().setCurrentPhysicsDeltaTime(newDeltaTimeStep);
	}
}

void Storm::SimulatorManager::addFluidParticleSystem(unsigned int id, std::vector<Storm::Vector3> particlePositions)
{
	LOG_COMMENT << "Creating fluid particle system with " << particlePositions.size() << " particles.";

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
	return this->getParticleSystem(id).getPositions();
}

float Storm::SimulatorManager::getKernelLength() const
{
	const Storm::GeneralSimulationData &generalSimulData = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>().getGeneralSimulationData();
	return generalSimulData._particleRadius * generalSimulData._kernelCoefficient;
}

void Storm::SimulatorManager::pushParticlesToGraphicModule(bool ignoreDirty) const
{
	Storm::IGraphicsManager &graphicMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IGraphicsManager>();
	std::for_each(std::execution::par, std::begin(_particleSystem), std::end(_particleSystem), [&graphicMgr, ignoreDirty](const auto &particleSystemPair)
	{
		const Storm::ParticleSystem &currentParticleSystem = *particleSystemPair.second;
		if (ignoreDirty || currentParticleSystem.isDirty())
		{
			graphicMgr.pushParticlesData(particleSystemPair.first, currentParticleSystem.getPositions(), currentParticleSystem.isFluids());
		}
	});
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
