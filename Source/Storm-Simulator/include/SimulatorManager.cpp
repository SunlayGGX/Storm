#include "SimulatorManager.h"

#include "SingletonHolder.h"
#include "IPhysicsManager.h"
#include "IThreadManager.h"
#include "IGraphicsManager.h"
#include "ITimeManager.h"
#include "TimeWaitResult.h"

#include "ParticleSystem.h"
#include "FluidParticleSystem.h"
#include "RigidBodyParticleSystem.h"


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



		// Update the Rigid bodies positions in scene
		physicsMgr.update(physicsElapsedDeltaTime);


		// Semi implicit Euler to update the particle position
		for (auto &particleSystem : _particleSystem)
		{
			particleSystem.second->updatePosition(physicsElapsedDeltaTime);
		}


		// Push all particle data to the graphic module to be rendered...
		// The first 4 frames every 1024 frames will be pushed for sure to the graphic module to be sure everyone is sync... 
		this->pushParticlesToGraphicModule(_forcedPushFrameIterator._val < 4);


		// Takes time to process messages that came from other threads.
		threadMgr.processCurrentThreadActions();

		++_forcedPushFrameIterator._val;

	} while (true);
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
	if (const auto foundParticleSystem = _particleSystem.find(id); foundParticleSystem != std::end(_particleSystem))
	{
		return foundParticleSystem->second->getPositions();
	}
	else
	{
		Storm::throwException<std::exception>("Particle system with id " + std::to_string(id) + " is unknown!");
	}
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
