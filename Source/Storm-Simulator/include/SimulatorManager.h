#pragma once


#include "Singleton.h"
#include "ISimulatorManager.h"
#include "DeclareScriptableItem.h"

#include "SingletonDefaultImplementation.h"

#include "ParticleSelector.h"

#include "ParticleSystemContainer.h"
#include "KernelHandler.h"


namespace Storm
{
	class ParticleSystem;
	class IBlower;
	class ISPHBaseSolver;
	class UIFieldContainer;
	class Cage;
	struct SceneSimulationConfig;
	struct SerializeRecordPendingData;
	enum class RaycastEnablingFlag : uint8_t;
	enum class SimulationSystemsState : uint8_t;

	class SimulatorManager final :
		private Storm::Singleton<Storm::SimulatorManager, Storm::DefineDefaultCleanupImplementationOnly>,
		public Storm::ISimulatorManager
	{
		STORM_DECLARE_SINGLETON(SimulatorManager);
		STORM_IS_SCRIPTABLE_ITEM;

	private:
		void initialize_Implementation();

	public:
		Storm::ExitCode run();
		
	private:
		Storm::ExitCode runSimulation_Internal();
		Storm::ExitCode runReplay_Internal();

	private:
		// CFL : Courant-Friedrich-Levy
		bool applyCFLIfNeeded(const Storm::SceneSimulationConfig &sceneSimulationConfig);

	private:
		void initializePreSimulation();

	public:
		void subIterationStart();
		void revertIteration();

		void advanceOneFrame();
		void advanceByFrame(int64_t frameCount);
		void advanceToFrame(int64_t frameNumber);

	private:
		void notifyFrameAdvanced();

	private:
		void evaluateCurrentSystemsState();

		void notifyCurrentSimulationStatesChanged();
		void onSystemStateIdle();
		void onSystemStateNormal();
		void onSystemStateUnstable();

	public:
		void flushPhysics(const float deltaTime);

	public:
		void refreshParticlesPosition() final override;
		void refreshParticleNeighborhood();

		void onGraphicParticleSettingsChanged() final override;

	public:
		void addFluidParticleSystem(unsigned int id, std::vector<Storm::Vector3> particlePositions) final override;
		void addRigidBodyParticleSystem(unsigned int id, std::vector<Storm::Vector3> particlePositions) final override;

		void addFluidParticleSystem(Storm::SystemSimulationStateObject &&state) final override;
		void addRigidBodyParticleSystem(Storm::SystemSimulationStateObject &&state) final override;

		void addFluidParticleSystem(unsigned int id, const std::size_t particleCount) final override;
		void addRigidBodyParticleSystem(unsigned int id, const std::size_t particleCount) final override;

		std::vector<Storm::Vector3> getParticleSystemPositions(unsigned int id) const final override;
		const std::vector<Storm::Vector3>& getParticleSystemPositionsReferences(unsigned int id) const final override;

	public:
		void loadBlower(const Storm::SceneBlowerConfig &blowerConfig) final override;
		void tweekBlowerEnabling();
		void advanceBlowersTime(const float deltaTime);
		void setBlowersStateTime(float blowerStateTime) final override;

	public:
		void setEnableThresholdDensity_DFSPH(bool enable);
		void setNeighborThresholdDensity_DFSPH(std::size_t neighborCount);

	public:
		void tweekRaycastEnabling();

	public:
		void printFluidParticleData() const final override;

	public:
		float getKernelLength() const final override;

	private:
		void pushParticlesToGraphicModule(bool ignoreDirty) const;

	private:
		void cycleSelectedParticleDisplayMode();
		void refreshParticleSelection();

	public:
		void requestCycleColoredSetting();

	public:
		void exitWithCode(const Storm::ExitCode code) final override;

	public:
		void beginRecord() const;
		void pushRecord(float currentPhysicsTime, bool pushStatics) const;

	private:
		void applyReplayFrame(Storm::SerializeRecordPendingData &frame, const float replayFps, bool pushParallel = true);

	public:
		void resetReplay();

	private:
		void resetReplay_SimulationThread();

	public:
		void saveSimulationState() const final override;

	private:
		void refreshParticlePartition(bool ignoreStatics = true) const;

	public:
		// Not from interface because they are intended to be used within simulation only (non thread safe)!
		Storm::ParticleSystem& getParticleSystem(unsigned int id);
		const Storm::ParticleSystem& getParticleSystem(unsigned int id) const;

	private:
		void executeAllForcesCheck();
		void printRigidBodyMoment(const unsigned int id) const;
		void printRigidBodyGlobalDensity(const unsigned int id) const;
		void printMassForRbDensity(const unsigned int id, const float wantedDensity);

	private:
		void logAverageDensity() const;
		void logVelocityData() const;
		void logTotalVolume() const;

	private:
		Storm::ParticleSystemContainer _particleSystem;
		std::vector<std::unique_ptr<Storm::IBlower>> _blowers;

		std::unique_ptr<Storm::ISPHBaseSolver> _sphSolver;
		Storm::KernelHandler _kernelHandler;

		Storm::ParticleSelector _particleSelector;
		Storm::RaycastEnablingFlag _raycastFlag;

		std::unique_ptr<Storm::Cage> _cage;

		Storm::SimulationSystemsState _currentSimulationSystemsState;
		float _maxVelocitySquaredLastStateCheck;
		std::vector<float> _tmp; // Temporary buffer for any work

		Storm::ExitCode _runExitCode;

		int64_t _currentFrameNumber;
		int64_t _frameAdvanceCount;

		// For replay
		std::unique_ptr<Storm::SerializeRecordPendingData> _frameBefore;
		bool _reinitFrameAfter;

		std::wstring _progressRemainingTime;

		std::unique_ptr<Storm::UIFieldContainer> _uiFields;
	};
}
