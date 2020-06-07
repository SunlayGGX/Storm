#include "SimulatorManager.h"

#include "SingletonHolder.h"
#include "IPhysicsManager.h"
#include "IThreadManager.h"
#include "ITimeManager.h"
#include "TimeWaitResult.h"

#include "FluidParticleSystem.h"


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

		// TODO : Run SPH

		physicsMgr.update(physicsElapsedDeltaTime);

		threadMgr.processCurrentThreadActions();

	} while (true);
}

void Storm::SimulatorManager::addParticleSystem(unsigned int id, std::vector<Storm::Vector3> particlePositions)
{
	auto fluidParticleSystemPtr = std::make_unique<Storm::FluidParticleSystem>(std::move(particlePositions));
	_fluidParticles[id] = std::move(fluidParticleSystemPtr);
}
