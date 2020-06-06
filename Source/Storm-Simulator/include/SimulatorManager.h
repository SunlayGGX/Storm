#pragma once


#include "Singleton.h"
#include "ISimulatorManager.h"


namespace Storm
{
	class SimulatorManager :
		private Storm::Singleton<SimulatorManager>,
		public Storm::ISimulatorManager
	{
		STORM_DECLARE_SINGLETON(SimulatorManager);

	private:
		void initialize_Implementation();
		void cleanUp_Implementation();

	public:
		void run();

	public:
		void executeOnSimulationLoop(Storm::SimulationCallback func) final override;
		void clearSimulationLoopCallback() final override;

	private:
		void handleSimulationCallbacks(std::vector<Storm::SimulationCallback> &tmpBuffer);

	private:
		mutable std::mutex _callbackMutex;
		std::vector<Storm::SimulationCallback> _simulationCallbacks;
	};
}
