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

#include "SimulationMode.h"

#include "DensitySolver.h"
#include "ViscositySolver.h"
#include "SemiImplicitEulerSolver.h"
#include "SpikyKernel.h"


namespace
{
	template<class ParticleSystemType, class MapType, class ... Args>
	void addParticleSystemToMap(MapType &map, unsigned int particleSystemId, Args &&... args)
	{
		std::unique_ptr<Storm::ParticleSystem> particleSystemPtr = std::make_unique<ParticleSystemType>(particleSystemId, std::forward<Args>(args)...);
		map[particleSystemId] = std::move(particleSystemPtr);
	}
}


Storm::SimulatorManager::SimulatorManager() = default;
Storm::SimulatorManager::~SimulatorManager() = default;

void Storm::SimulatorManager::initialize_Implementation()
{
	// TODO
}

void Storm::SimulatorManager::cleanUp_Implementation()
{
	// TODO
}

void Storm::SimulatorManager::run()
{
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

				// Pressure init (heuristically)
				pressures[currentPIndex] = std::max(20000.f * (currentPDensity - k_particleSystemRestDensity), 0.f);
			});

			// Viscosity
			std::for_each(std::execution::par, std::begin(forces), std::end(forces), [&](Storm::Vector3 &force)
			{
				const std::size_t currentPIndex = Storm::ParticleSystem::getParticleIndex(forces, force);
				force += Storm::ViscositySolver::computeViscosityForcePCISPH(k_massPerParticle, densities[currentPIndex], velocities[currentPIndex], neighborhoodArrays[currentPIndex]);

				// TODO : reflect the force on each neighborhood (need to change a little the architecture)
			});
		}
	}

	const Storm::GradientSpikyKernel gradSpikyKernel{ this->getKernelLength() };

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

						for (const Storm::NeighborParticleInfo &pNeighbor : currentPNeighbors)
						{
							if (pNeighbor._isFluidParticle)
							{
								const Storm::Vector3 &predictedNeighborPPosition = pNeighbor._containingParticleSystem->getPredictedPositions()[pNeighbor._particleIndex];
								Storm::Vector3 diffPositionVectorXij = currentPPredictedPosition - predictedNeighborPPosition;
								kernelGradSum += gradSpikyKernel(diffPositionVectorXij.norm());
							}
							else
							{
								const float neighborDensity = pNeighbor._containingParticleSystem->getDensities()[pNeighbor._particleIndex];
								kCoeff += k_particleSystemRestDensity * neighborDensity * gradSpikyKernel(pNeighbor._vectToParticleNorm);
							}
						}

						float &currentPPredictedDensity = predictedDensities[currentPIndex];
						currentPPredictedDensity = (k_massPerParticle * kernelGradSum) + kCoeff;

						const float currentPDensityError = currentPPredictedDensity - k_particleSystemRestDensity;
						runPredictionTmp = std::abs(currentPDensityError) > k_maxDensityError || runPredictionTmp;

						currentPPredictedPressure += currentPDensityError * k_invertPhysicsTimeSquared;
						if (currentPPredictedPressure < 0.f)
						{
							currentPPredictedPressure = 0.f;
						}
					});

					runPrediction = runPredictionTmp;
				}
			}

			// Third : Compute the predicted pressure force
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

						for (const Storm::NeighborParticleInfo &pNeighbor : currentPNeighbors)
						{
							const float gradientCoeff = gradSpikyKernel(pNeighbor._vectToParticleNorm);
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

							currentPPredictedPressure += (coeff / pNeighbor._vectToParticleNorm) * pNeighbor._positionDifferenceVector;

#if false
							// TODO : reflect the force on each neighborhood (need to change a little the architecture)
							if (!runPrediction && !pNeighbor._isFluidParticle)
							{

							}
#endif
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
