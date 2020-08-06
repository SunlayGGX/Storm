#pragma once


#include "Singleton.h"
#include "ISimulatorManager.h"


namespace Storm
{
	class ParticleSystem;
	struct GeneralSimulationData;

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

	private:
		void executeWCSPH(float physicsElapsedDeltaTime);
		void executePCISPH(float physicsElapsedDeltaTime);

	private:
		// CFL : Courant-Friedrich-Levy
		void applyCFLIfNeeded(const Storm::GeneralSimulationData &generalSimulationDataConfig);

	public:
		void addFluidParticleSystem(unsigned int id, std::vector<Storm::Vector3> particlePositions) final override;
		void addRigidBodyParticleSystem(unsigned int id, std::vector<Storm::Vector3> particlePositions) final override;

		std::vector<Storm::Vector3> getParticleSystemPositions(unsigned int id) const final override;

	public:
		void printFluidParticleData() const final override;

	public:
		float getKernelLength() const final override;

	private:
		void pushParticlesToGraphicModule(bool ignoreDirty) const;

	public:
		// Not from interface because they are intended to be used within simulation only (non thread safe)!
		Storm::ParticleSystem& getParticleSystem(unsigned int id);
		const Storm::ParticleSystem& getParticleSystem(unsigned int id) const;

	private:
		std::map<unsigned int, std::unique_ptr<Storm::ParticleSystem>> _particleSystem;
	};
}
