#include "SimulatorManager.h"

#include "SingletonHolder.h"
#include "ITimeManager.h"
#include "TimeWaitResult.h"


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
	Storm::ITimeManager* timeMgr = Storm::SingletonHolder::instance().getFacet<Storm::ITimeManager>();
	
	while (true)
	{
		Storm::TimeWaitResult simulationState = timeMgr->waitNextFrame();
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

		// TODO : Run the physics simulation
	}
}
