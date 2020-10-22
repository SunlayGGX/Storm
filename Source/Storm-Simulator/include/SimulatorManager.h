#pragma once


#include "Singleton.h"
#include "ISimulatorManager.h"

#include "ParticleSelector.h"

#include "ParticleSystemContainer.h"


namespace Storm
{
	class ParticleSystem;
	class IBlower;
	class ISPHBaseSolver;
	struct GeneralSimulationData;
	struct SerializeRecordPendingData;

	class SimulatorManager :
		private Storm::Singleton<SimulatorManager>,
		public Storm::ISimulatorManager
	{
		STORM_DECLARE_SINGLETON(SimulatorManager);

	private:
		void initialize_Implementation();
		void cleanUp_Implementation();

	public:
		Storm::ExitCode run();
		
	private:
		Storm::ExitCode runSimulation_Internal();
		Storm::ExitCode runReplay_Internal();

		void executeIteration(bool firstFrame, unsigned int forcedPushFrameIterator);

	private:
		// CFL : Courant-Friedrich-Levy
		bool applyCFLIfNeeded(const Storm::GeneralSimulationData &generalSimulationDataConfig);

	private:
		void initializePreSimulation();

	public:
		void refreshParticlesPosition() final override;

	public:
		void addFluidParticleSystem(unsigned int id, std::vector<Storm::Vector3> particlePositions) final override;
		void addRigidBodyParticleSystem(unsigned int id, std::vector<Storm::Vector3> particlePositions) final override;

		void addFluidParticleSystem(unsigned int id, const std::size_t particleCount) final override;
		void addRigidBodyParticleSystem(unsigned int id, const std::size_t particleCount) final override;

		std::vector<Storm::Vector3> getParticleSystemPositions(unsigned int id) const final override;
		const std::vector<Storm::Vector3>& getParticleSystemPositionsReferences(unsigned int id) const final override;

	public:
		void loadBlower(const Storm::BlowerData &blowerData) final override;
		void tweekBlowerEnabling();

	public:
		void tweekRaycastEnabling();

	public:
		void printFluidParticleData() const final override;

	public:
		float getKernelLength() const final override;

	private:
		void pushParticlesToGraphicModule(bool ignoreDirty, bool pushParallel = true) const;

	private:
		void cycleSelectedParticleDisplayMode();

	public:
		void exitWithCode(Storm::ExitCode code) final override;

	public:
		void beginRecord() const;
		void pushRecord(float currentPhysicsTime, bool pushStatics) const;

	public:
		void resetReplay();

	private:
		void refreshParticlePartition(bool ignoreStatics = true) const;

	public:
		// Not from interface because they are intended to be used within simulation only (non thread safe)!
		Storm::ParticleSystem& getParticleSystem(unsigned int id);
		const Storm::ParticleSystem& getParticleSystem(unsigned int id) const;

	private:
		Storm::ParticleSystemContainer _particleSystem;
		std::vector<std::unique_ptr<Storm::IBlower>> _blowers;

		std::unique_ptr<Storm::ISPHBaseSolver> _sphSolver;

		Storm::ParticleSelector _particleSelector;
		bool _raycastEnabled;

		Storm::ExitCode _runExitCode;

		// For replay
		std::unique_ptr<Storm::SerializeRecordPendingData> _frameBefore;
		bool _reinitFrameAfter;
	};
}
