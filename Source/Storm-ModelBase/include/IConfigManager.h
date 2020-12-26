#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	struct SceneData;
	struct GraphicData;
	struct RigidBodySceneData;
	struct FluidData;
	struct BlowerData;
	struct ConstraintData;
	struct GeneralSimulationData;
	struct RecordConfigData;
	struct ScriptData;

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

		// Logs
		virtual const std::string& getLogFileName() const = 0;
		virtual const std::string& getLogFolderPath() const = 0;
		virtual Storm::LogLevel getLogLevel() const = 0;
		virtual int getRemoveLogOlderThanDaysCount() const = 0;
		virtual bool getShouldOverrideOldLog() const = 0;
		virtual bool getShouldLogFpsWatching() const = 0;
		virtual bool getShouldLogGraphicDeviceMessage() const = 0;
		virtual bool getShouldLogPhysics() const = 0;
		virtual bool noPopup() const = 0;

		// Debug
		virtual Storm::VectoredExceptionDisplayMode getVectoredExceptionsDisplayMode() const = 0;

		// General Graphics
		virtual unsigned int getWantedScreenWidth() const = 0;
		virtual unsigned int getWantedScreenHeight() const = 0;
		virtual int getWantedScreenXPosition() const = 0;
		virtual int getWantedScreenYPosition() const = 0;
		virtual float getFontSize() const = 0;
		virtual bool getFixNearFarPlanesWhenTranslatingFlag() const = 0;
		virtual bool getSelectedParticleShouldBeTopMost() const = 0;
		virtual bool getSelectedParticleForceShouldBeTopMost() const = 0;

		// Profile
		virtual bool getShouldProfileSimulationSpeed() const = 0;

		// Misc
		virtual bool urlOpenIncognito() const = 0;

		// Scene data
		virtual const Storm::SceneData& getSceneData() const = 0;
		virtual const Storm::GraphicData& getGraphicData() const = 0;
		virtual const Storm::GeneralSimulationData& getGeneralSimulationData() const = 0;
		virtual const std::vector<Storm::RigidBodySceneData>& getRigidBodiesData() const = 0;
		virtual const Storm::FluidData& getFluidData() const = 0;
		virtual const Storm::RecordConfigData& getRecordConfigData() const = 0;
		virtual const std::vector<Storm::BlowerData>& getBlowersData() const = 0;
		virtual const std::vector<Storm::ConstraintData>& getConstraintsData() const = 0;
		virtual const Storm::RigidBodySceneData& getRigidBodyData(unsigned int rbId) const = 0;
		virtual const Storm::ScriptData& getScriptData() const = 0;

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
