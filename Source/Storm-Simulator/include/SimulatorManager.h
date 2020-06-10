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

		std::vector<Storm::Vector3> getParticleSystemPositions(unsigned int id) const final override;

	private:
		void pushParticlesToGraphicModule(bool ignoreDirty) const;

	private:
		std::map<unsigned int, std::unique_ptr<Storm::ParticleSystem>> _particleSystem;
	};
}
