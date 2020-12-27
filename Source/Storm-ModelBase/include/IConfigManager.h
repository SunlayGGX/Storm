#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	struct GeneralGraphicConfig;
	struct GeneralSimulationConfig;
	struct GeneralWebConfig;
	struct GeneralDebugConfig;

	struct SceneConfig;
	struct SceneGraphicConfig;
	struct SceneRigidBodyConfig;
	struct SceneFluidConfig;
	struct SceneBlowerConfig;
	struct SceneConstraintConfig;
	struct SceneSimulationConfig;
	struct SceneRecordConfig;
	struct SceneScriptConfig;

	enum class ThreadPriority;
	enum class VectoredExceptionDisplayMode;

	class IConfigManager : public Storm::ISingletonHeldInterface<IConfigManager>
	{
	public:
		virtual ~IConfigManager() = default;

	public:
		// Paths
		virtual const std::string& getTemporaryPath() const = 0;
		virtual const std::string& getExePath() const = 0;

		// Command line
		virtual bool shouldRegenerateParticleCache() const = 0;
		virtual bool withUI() const = 0;
		virtual Storm::ThreadPriority getUserSetThreadPriority() const = 0;
		virtual const std::string& getStateFilePath() const = 0;
		virtual void stateShouldLoad(bool &outLoadPhysicsTime, bool &outLoadForces, bool &outLoadVelocities) const = 0;

		// General config
		virtual const Storm::GeneralGraphicConfig& getGeneralGraphicConfig() const = 0;
		virtual const Storm::GeneralSimulationConfig& getGeneralSimulationConfig() const = 0;
		virtual const Storm::GeneralWebConfig& getGeneralWebConfig() const = 0;
		virtual const Storm::GeneralDebugConfig& getGeneralDebugConfig() const = 0;

		// Scene config
		virtual const Storm::SceneGraphicConfig& getSceneGraphicConfig() const = 0;
		virtual const Storm::SceneSimulationConfig& getSceneSimulationConfig() const = 0;
		virtual const std::vector<Storm::SceneRigidBodyConfig>& getSceneRigidBodiesConfig() const = 0;
		virtual const Storm::SceneFluidConfig& getSceneFluidConfig() const = 0;
		virtual const Storm::SceneRecordConfig& getSceneRecordConfig() const = 0;
		virtual const std::vector<Storm::SceneBlowerConfig>& getSceneBlowersConfig() const = 0;
		virtual const std::vector<Storm::SceneConstraintConfig>& getSceneConstraintsConfig() const = 0;
		virtual const Storm::SceneRigidBodyConfig& getSceneRigidBodyConfig(unsigned int rbId) const = 0;
		virtual const Storm::SceneScriptConfig& getSceneScriptConfig() const = 0;

		// Special highly used method
		virtual bool isInReplayMode() const noexcept = 0;
		virtual bool userCanModifyTimestep() const noexcept = 0;
		virtual const std::string& getSceneName() const = 0;
		virtual const std::string& getSimulationTypeName() const = 0;
		virtual unsigned int getCurrentPID() const = 0; // PID = Process ID
		virtual const std::string& getSceneConfigFilePath() const = 0;
		virtual const std::string& getScriptFilePath() const = 0;
	};
}
