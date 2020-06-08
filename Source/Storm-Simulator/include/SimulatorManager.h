#pragma once


#include "Singleton.h"
#include "ISimulatorManager.h"


namespace Storm
{
	class ParticleSystem;

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
		void addFluidParticleSystem(unsigned int id, std::vector<Storm::Vector3> particlePositions) final override;
		void addRigidBodyParticleSystem(unsigned int id, std::vector<Storm::Vector3> particlePositions) final override;

	private:
		void pushParticlesToGraphicModule() const;

	private:
		std::map<unsigned int, std::unique_ptr<Storm::ParticleSystem>> _particleSystem;
	};
}
