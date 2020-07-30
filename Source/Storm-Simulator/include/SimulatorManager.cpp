#include "SimulatorManager.h"

#include "SingletonHolder.h"
#include "IPhysicsManager.h"
#include "IThreadManager.h"
#include "IGraphicsManager.h"
#include "IConfigManager.h"
#include "IInputManager.h"
#include "ITimeManager.h"
#include "TimeWaitResult.h"

#include "ParticleSystem.h"
#include "FluidParticleSystem.h"
#include "RigidBodyParticleSystem.h"

#include "GeneralSimulationData.h"
#include "FluidData.h"

#include "SimulationMode.h"
#include "KernelMode.h"

#include "SemiImplicitEulerSolver.h"
#include "Kernel.h"

#include "SpecialKey.h"

#include "RunnerHelper.h"

#include <fstream>


namespace
{
	template<class ParticleSystemType, class MapType, class ... Args>
	void addParticleSystemToMap(MapType &map, unsigned int particleSystemId, Args &&... args)
	{
		std::unique_ptr<Storm::ParticleSystem> particleSystemPtr = std::make_unique<ParticleSystemType>(particleSystemId, std::forward<Args>(args)...);
		map[particleSystemId] = std::move(particleSystemPtr);
	}

	class Vector3Parser
	{
	public:
		template<class PolicyType>
		static std::string parse(const Storm::Vector3 &vect)
		{
			return "{ " + std::to_string(vect.x()) + ',' + std::to_string(vect.y()) + ',' + std::to_string(vect.z()) + " }";
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

		stream << dataName << " :\n\n" << Storm::toStdString<Vector3Parser>(dataContainer) << "\n\n";
	}
}


Storm::SimulatorManager::SimulatorManager() = default;
Storm::SimulatorManager::~SimulatorManager() = default;

void Storm::SimulatorManager::initialize_Implementation()
{
	/* Initialize kernels */

	Storm::initializeKernels(this->getKernelLength());

	Storm::IInputManager &inputMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IInputManager>();
	inputMgr.bindKey(Storm::SpecialKey::KC_F1, [this]() { this->printFluidParticleData(); });

	// First position update to regenerate the position of any particle according to its translation.
	for (auto &particleSystem : _particleSystem)
	{
		particleSystem.second->updatePosition(0.f);
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
			if (_forcedPushFrameIterator._val < 5)
			{
				this->pushParticlesToGraphicModule(true);
				++_forcedPushFrameIterator._val;
			}

			// Takes time to process messages that came from other threads.
			threadMgr.processCurrentThreadActions();
			continue;

		case TimeWaitResult::Continue:
		default:
			break;
		}

		const float physicsElapsedDeltaTime = timeMgr.getCurrentPhysicsDeltaTime();

		// initialize for current iteration. I.e. Initializing with gravity and resetting current iteration velocity.
		// Also build neighborhood.
		// TODO: Optimize
		for (auto &particleSystem : _particleSystem)
		{
			particleSystem.second->initializeIteration(_particleSystem);
		}

		// Compute the simulation
		switch (generalSimulationConfigData._simulationMode)
		{
		case Storm::SimulationMode::SESPH:
			this->executeSESPH(physicsElapsedDeltaTime);
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
	const Storm::GeneralSimulationData &generalSimulData = configMgr.getGeneralSimulationData();
	const Storm::FluidData &fluidConfigData = configMgr.getFluidData();

	const float k_kernelLength = this->getKernelLength();
	const float k_kernelZero = Storm::retrieveKernelZeroValue(generalSimulData._kernelMode);
	const Storm::RawKernelMethodDelegate rawKernel = Storm::retrieveRawKernelMethod(generalSimulData._kernelMode);
	const Storm::GradKernelMethodDelegate gradKernel = Storm::retrieveGradKernelMethod(generalSimulData._kernelMode);

	// First : compute densities and pressure data
	for (auto &particleSystemPair : _particleSystem)
	{
		Storm::ParticleSystem &currentParticleSystem = *particleSystemPair.second;
		if (currentParticleSystem.isFluids())
		{
			Storm::FluidParticleSystem &fluidParticleSystem = dynamic_cast<Storm::FluidParticleSystem &>(currentParticleSystem);

			const float particleVolume = fluidParticleSystem.getParticleVolume();
			const float density0 = fluidParticleSystem.getRestDensity();
			const std::vector<Storm::ParticleNeighborhoodArray> &neighborhoodArrays = fluidParticleSystem.getNeighborhoodArrays();
			std::vector<float> &pressures = fluidParticleSystem.getPressures();

			Storm::runParallel(fluidParticleSystem.getDensities(), [&](float &currentPDensity, const std::size_t currentPIndex)
			{
				// Density
				currentPDensity = particleVolume * k_kernelZero;

				const Storm::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoodArrays[currentPIndex];
				for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
				{
					const float kernelValue_Wij = rawKernel(k_kernelLength, neighbor._vectToParticleNorm);
					float deltaDensity;
					if (neighbor._isFluidParticle)
					{
						deltaDensity = static_cast<Storm::FluidParticleSystem*>(neighbor._containingParticleSystem)->getParticleVolume() * kernelValue_Wij;
					}
					else
					{
						deltaDensity = static_cast<Storm::RigidBodyParticleSystem*>(neighbor._containingParticleSystem)->getVolumes()[neighbor._particleIndex] * kernelValue_Wij;
					}
					currentPDensity += deltaDensity;
				}

				// Volume * density is mass...
				currentPDensity *= density0;
				if (currentPDensity < density0)
				{
					currentPDensity = density0;
				}

				// Pressure
				float &currentPPressure = pressures[currentPIndex];
				currentPPressure = fluidConfigData._kPressureStiffnessCoeff * (std::powf(currentPDensity / density0, fluidConfigData._kPressureExponentCoeff) - 1.f);
			});
		}
	}

	// Second : Compute forces : pressure and viscosity
	for (auto &particleSystemPair : _particleSystem)
	{
		Storm::ParticleSystem &currentParticleSystem = *particleSystemPair.second;
		if (currentParticleSystem.isFluids())
		{
			Storm::FluidParticleSystem &fluidParticleSystem = static_cast<Storm::FluidParticleSystem &>(currentParticleSystem);

			const float density0 = fluidParticleSystem.getRestDensity();
			const float density0Squared = density0 * density0;
			const std::vector<Storm::ParticleNeighborhoodArray> &neighborhoodArrays = currentParticleSystem.getNeighborhoodArrays();
			const std::vector<float> &masses = fluidParticleSystem.getMasses();
			const std::vector<float> &densities = fluidParticleSystem.getDensities();
			const std::vector<float> &pressures = fluidParticleSystem.getPressures();
			const std::vector<Storm::Vector3> &velocities = fluidParticleSystem.getVelocity();

			Storm::runParallel(fluidParticleSystem.getForces(), [&](Storm::Vector3 &currentPForce, const std::size_t currentPIndex)
			{
				const float currentPMass = masses[currentPIndex];
				const float currentPDensity = densities[currentPIndex];
				const float currentPPressure = pressures[currentPIndex];
				const Storm::Vector3 &vi = velocities[currentPIndex];

				const float currentPFluidPressureCoeff = currentPPressure / (currentPDensity * currentPDensity);

				const float restMassDensity = currentPMass * density0;
				const float currentPRestMassDensityBoundaryPressureCoeff = restMassDensity * (currentPFluidPressureCoeff + (currentPPressure / density0Squared));

				const float viscoPrecoeff = 0.01f * k_kernelLength * k_kernelLength;

				const Storm::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoodArrays[currentPIndex];
				for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
				{
					const Storm::Vector3 gradKernel_NablaWij = gradKernel(k_kernelLength, neighbor._positionDifferenceVector, neighbor._vectToParticleNorm);

					const Storm::Vector3 vij = vi - neighbor._containingParticleSystem->getVelocity()[neighbor._particleIndex];

					const float vijDotXij = vij.dot(neighbor._positionDifferenceVector);
					const float viscoGlobalCoeff = currentPMass * 10.f * vijDotXij / (neighbor._vectToParticleSquaredNorm + viscoPrecoeff);

					Storm::Vector3 forceAdded = Storm::Vector3::Zero();

					if (neighbor._isFluidParticle)
					{
						const Storm::FluidParticleSystem* neighborPSystemAsFluid = static_cast<Storm::FluidParticleSystem*>(neighbor._containingParticleSystem);
						const float neighborDensity0 = neighborPSystemAsFluid->getRestDensity();
						const float neighborMass = neighborPSystemAsFluid->getMasses()[neighbor._particleIndex];
						const float neighborRawDensity = neighborPSystemAsFluid->getDensities()[neighbor._particleIndex];
						const float neighborDensity = neighborRawDensity * density0 / neighborDensity0;
						const float neighborVolume = neighborPSystemAsFluid->getParticleVolume();

						// Pressure
						const float neighborPressureCoeff = neighborPSystemAsFluid->getPressures()[neighbor._particleIndex] / (neighborDensity * neighborDensity);
						forceAdded -= (restMassDensity * neighborVolume * (currentPFluidPressureCoeff + neighborPressureCoeff)) * gradKernel_NablaWij;

						// Viscosity
						forceAdded += (viscoGlobalCoeff * fluidConfigData._dynamicViscosity * neighborMass / neighborRawDensity) * gradKernel_NablaWij;
					}
					else
					{
						const Storm::RigidBodyParticleSystem* neighborPSystemAsBoundary = static_cast<Storm::RigidBodyParticleSystem*>(neighbor._containingParticleSystem);

						const float neighborVolume = neighborPSystemAsBoundary->getVolumes()[neighbor._particleIndex];
						const float rbViscosity = neighborPSystemAsBoundary->getViscosity();

						// Pressure
						forceAdded -= (currentPRestMassDensityBoundaryPressureCoeff * neighborVolume) * gradKernel_NablaWij;

						// Viscosity
						if (rbViscosity > 0.f)
						{
							forceAdded += (viscoGlobalCoeff * rbViscosity * neighborVolume * density0 / currentPDensity) * gradKernel_NablaWij;
						}

						// Mirror the force on the boundary solid following the 3rd newton law
						if (!neighborPSystemAsBoundary->isStatic())
						{
							Storm::Vector3 &boundaryNeighborForce = neighbor._containingParticleSystem->getForces()[neighbor._particleIndex];

							std::lock_guard<std::mutex> lock{ neighbor._containingParticleSystem->_mutex };
							boundaryNeighborForce -= forceAdded;
						}
					}

					currentPForce += forceAdded;
				}
			});
		}
	}
}

void Storm::SimulatorManager::executePCISPH(float physicsElapsedDeltaTime)
{
	STORM_NOT_IMPLEMENTED;
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
	Storm::runParallel(_particleSystem, [&graphicMgr, ignoreDirty](const auto &particleSystemPair)
	{
		const Storm::ParticleSystem &currentParticleSystem = *particleSystemPair.second;
		if (ignoreDirty || currentParticleSystem.isDirty() || currentParticleSystem.isFluids())
		{
			graphicMgr.pushParticlesData(particleSystemPair.first, currentParticleSystem.getPositions(), currentParticleSystem.getVelocity(), currentParticleSystem.isFluids(), currentParticleSystem.isWall());
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
