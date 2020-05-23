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
	};
}
