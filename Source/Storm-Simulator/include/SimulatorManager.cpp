#include "SimulatorManager.h"

#include "SingletonHolder.h"
#include "ITimeManager.h"
#include "TimeWaitResult.h"


Storm::SimulatorManager::SimulatorManager()
{
	_simulationCallbacks.reserve(8);
}

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
	
	std::vector<Storm::SimulationCallback> tmpSimulationCallback;
	tmpSimulationCallback.reserve(8);

	do
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

		this->handleSimulationCallbacks(tmpSimulationCallback);
	} while (true);
}

void Storm::SimulatorManager::executeOnSimulationLoop(Storm::SimulationCallback func)
{
	std::lock_guard<std::mutex> lock{ _callbackMutex };
	_simulationCallbacks.emplace_back(std::move(func));
}

void Storm::SimulatorManager::handleSimulationCallbacks(std::vector<Storm::SimulationCallback> &tmpBuffer)
{
	{
		std::lock_guard<std::mutex> lock{ _callbackMutex };
		std::swap(_simulationCallbacks, tmpBuffer);
	}

	for (auto &callback : tmpBuffer)
	{
		try
		{
			callback();
		}
		catch (const std::exception &e)
		{
			LOG_ERROR << "Callback call in simulation loop triggered an exception : " << e.what();
		}
		catch (...)
		{
			LOG_ERROR << "Callback call in simulation loop triggered an unknown exception!";
		}
	}

	tmpBuffer.clear();
}
