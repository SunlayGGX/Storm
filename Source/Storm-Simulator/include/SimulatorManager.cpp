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
#include "CubicSplineKernel.h"

#include "RunnerHelper.h"


namespace
{
	using RawKernelMethodDelegate = float(*)(const float k_kernelLength, const float norm);
	using GradKernelMethodDelegate = Storm::Vector3(*)(const float, const Storm::Vector3 &, const float);

	template<class ParticleSystemType, class MapType, class ... Args>
	void addParticleSystemToMap(MapType &map, unsigned int particleSystemId, Args &&... args)
	{
		std::unique_ptr<Storm::ParticleSystem> particleSystemPtr = std::make_unique<ParticleSystemType>(particleSystemId, std::forward<Args>(args)...);
		map[particleSystemId] = std::move(particleSystemPtr);
	}

	RawKernelMethodDelegate retrieveRawKernelMethod(const Storm::KernelMode kernelMode)
	{
		switch (kernelMode)
		{
		case Storm::KernelMode::CubicSpline: return Storm::CubicSplineKernel::raw;
		}

		Storm::throwException<std::exception>("Unknown kernel mode!");
	}

	GradKernelMethodDelegate retrieveGradKernelMethod(const Storm::KernelMode kernelMode)
	{
		switch (kernelMode)
		{
		case Storm::KernelMode::CubicSpline: return Storm::CubicSplineKernel::gradient;
		}

		Storm::throwException<std::exception>("Unknown kernel mode!");
	}
}


Storm::SimulatorManager::SimulatorManager() = default;
Storm::SimulatorManager::~SimulatorManager() = default;

void Storm::SimulatorManager::initialize_Implementation()
{
	/* Initialize kernels */

	const float k_kernelLength = this->getKernelLength();

	Storm::CubicSplineKernel::initialize(k_kernelLength);
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
		case Storm::SimulationMode::SESPH:
			this->executePCISPH(physicsElapsedDeltaTime);
			break;

		case Storm::SimulationMode::PCISPH:
			this->executePCISPH(physicsElapsedDeltaTime);
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

void Storm::SimulatorManager::executeSESPH(float physicsElapsedDeltaTime)
{
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();

	const Storm::GeneralSimulationData &generalSimulationDataConfig = configMgr.getGeneralSimulationData();
	const Storm::FluidData &fluidSimulationDataConfig = configMgr.getFluidData();

	const float k_kernelLength = this->getKernelLength();
	const RawKernelMethodDelegate rawKernelMethod = retrieveRawKernelMethod(generalSimulationDataConfig._kernelMode);

	for (auto &particleSystemPair : _particleSystem)
	{
		Storm::ParticleSystem &currentParticleSystem = *particleSystemPair.second;

		if (currentParticleSystem.isFluids())
		{
			std::vector<Storm::Vector3> &forces = currentParticleSystem.getForces();
			std::vector<float> &densities = currentParticleSystem.getDensities();
			std::vector<float> &pressures = currentParticleSystem.getPressures();
			const std::vector<Storm::ParticleSystem::ParticleNeighborhoodArray> &neighborhoods = currentParticleSystem.getNeighborhoodArrays();
			const float k_massPerParticle = currentParticleSystem.getMassPerParticle();
			const float k_restDensity = currentParticleSystem.getRestDensity();

			const Storm::Vector3 gravityForce = k_massPerParticle * generalSimulationDataConfig._gravity;

			Storm::runParallel(forces, [&](Storm::Vector3 &currentPForce, const std::size_t currentPIndex)
			{
				// Gravity
				currentPForce += gravityForce;

				// TODO : Blower

				// Density
				float &currentPDensity = densities[currentPIndex];
				currentPDensity = 0.f;

				const Storm::ParticleSystem::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoods[currentPIndex];

				for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
				{
					const float kernelValue = rawKernelMethod(k_kernelLength, neighbor._vectToParticleNorm);
					currentPDensity += kernelValue;
				}

				currentPDensity *= k_massPerParticle;

				// Pressure
				float &currentPPressure = pressures[currentPIndex];
				const float densityRatio = currentPDensity / k_restDensity;
				currentPPressure = fluidSimulationDataConfig._kPressureCoeff * (densityRatio - 1.f);
			});
		}
	}

}

void Storm::SimulatorManager::executePCISPH(float physicsElapsedDeltaTime)
{
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();

	const Storm::GeneralSimulationData &generalSimulationDataConfig = configMgr.getGeneralSimulationData();
	const Storm::FluidData &fluidSimulationDataConfig = configMgr.getFluidData();

	const float k_kernelLength = this->getKernelLength();
	const RawKernelMethodDelegate rawKernelMethod = retrieveRawKernelMethod(generalSimulationDataConfig._kernelMode);

	for (auto &particleSystemPair : _particleSystem)
	{
		Storm::ParticleSystem &currentParticleSystem = *particleSystemPair.second;

		std::vector<Storm::Vector3> &forces = currentParticleSystem.getForces();
		std::vector<float> &densities = currentParticleSystem.getDensities();
		std::vector<float> &pressures = currentParticleSystem.getPressures();
		const std::vector<Storm::ParticleSystem::ParticleNeighborhoodArray> &neighborhoods = currentParticleSystem.getNeighborhoodArrays();
		const float k_massPerParticle = currentParticleSystem.getMassPerParticle();
		const float k_restDensity = currentParticleSystem.getRestDensity();
		const bool isFluid = currentParticleSystem.isFluids();

		Storm::runParallel(forces, [&](Storm::Vector3 &currentPForce, const std::size_t currentPIndex)
		{
			// TODO : Blower

			// Density
			float &currentPDensity = densities[currentPIndex];
			currentPDensity = 0.f;

			const Storm::ParticleSystem::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoods[currentPIndex];

			for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
			{
				const float kernelValue = rawKernelMethod(k_kernelLength, neighbor._vectToParticleNorm);
				currentPDensity += kernelValue;
			}

			currentPDensity *= k_massPerParticle;

			// Pressure
			float &currentPPressure = pressures[currentPIndex];
			const float densityRatio = currentPDensity / k_restDensity;
			currentPPressure = fluidSimulationDataConfig._kPressureCoeff * (densityRatio - 1.f);
		});
	}


	// Finally The predicted pressure (which isn't predicted anymore) should be added to the total force applied to the particle
	for (auto &particleSystemPair : _particleSystem)
	{
		particleSystemPair.second->flushPressureToTotalForce();
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
		/*constexpr float minDeltaTime = 0.0001f;
		if (newDeltaTimeStep < minDeltaTime)
		{
			newDeltaTimeStep = minDeltaTime;
		}*/

		if (newDeltaTimeStep > generalSimulationDataConfig._maxCFLTime)
		{
			newDeltaTimeStep = generalSimulationDataConfig._maxCFLTime;
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
