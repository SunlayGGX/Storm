#include "SimulatorManager.h"

#include "SingletonHolder.h"
#include "IPhysicsManager.h"
#include "IThreadManager.h"
#include "IGraphicsManager.h"
#include "ITimeManager.h"
#include "TimeWaitResult.h"

#include "ParticleSystem.h"


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

		this->pushParticlesToGraphicModule();

		threadMgr.processCurrentThreadActions();

	} while (true);
}

void Storm::SimulatorManager::addParticleSystem(unsigned int id, std::vector<Storm::Vector3> particlePositions)
{
	auto fluidParticleSystemPtr = std::make_unique<Storm::ParticleSystem>(std::move(particlePositions));
	_particleSystem[id] = std::move(fluidParticleSystemPtr);
}

void Storm::SimulatorManager::pushParticlesToGraphicModule() const
{
	Storm::IGraphicsManager &graphicMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IGraphicsManager>();
	std::for_each(std::execution::par, std::begin(_particleSystem), std::end(_particleSystem), [&graphicMgr](const auto &particleSystemPair)
	{
		graphicMgr.pushParticlesData(particleSystemPair.first, particleSystemPair.second->getPositions());
	});
}
