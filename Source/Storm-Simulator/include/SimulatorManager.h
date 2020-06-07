#pragma once


#include "Singleton.h"
#include "ISimulatorManager.h"


namespace Storm
{
	class FluidParticleSystem;

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
		void addParticleSystem(unsigned int id, std::vector<Storm::Vector3> particlePositions) final override;

	private:
		std::map<unsigned int, std::unique_ptr<Storm::FluidParticleSystem>> _fluidParticles;
	};
}
