#pragma once

#include "Singleton.h"
#include "IConfigManager.h"
#include "SingletonDefaultImplementation.h"

#include "MacroConfig.h"
#include "GeneralConfig.h"
#include "SceneConfigHolder.h"


namespace Storm
{
	class ConfigManager final :
		private Storm::Singleton<ConfigManager, Storm::DefineDefaultCleanupImplementationOnly>,
		public Storm::IConfigManager
	{
		STORM_DECLARE_SINGLETON(ConfigManager);

	private:
		void initialize_Implementation(int argc, const char* argv[]);

	public:
		const std::string& getTemporaryPath() const final override;
		const std::string& getLogFileName() const final override;
		const std::string& getExePath() const final override;

		bool shouldRegenerateParticleCache() const final override;

		bool withUI() const final override;

		Storm::ThreadPriority getUserSetThreadPriority() const final override;

		const std::string& getStateFilePath() const final override;
		void stateShouldLoad(bool &outLoadPhysicsTime, bool &outLoadForces, bool &outLoadVelocities) const final override;

		const std::string& getLogFolderPath() const final override;
		Storm::LogLevel getLogLevel() const final override;
		int getRemoveLogOlderThanDaysCount() const final override;
		bool getShouldOverrideOldLog() const final override;
		bool getShouldLogFpsWatching() const final override;
		bool getShouldLogGraphicDeviceMessage() const final override;
		bool getShouldLogPhysics() const final override;
		bool noPopup() const final override;

		Storm::VectoredExceptionDisplayMode getVectoredExceptionsDisplayMode() const final override;
		
		bool getShouldProfileSimulationSpeed() const final override;

		bool urlOpenIncognito() const final override;

		unsigned int getWantedScreenWidth() const final override;
		unsigned int getWantedScreenHeight() const final override;
		int getWantedScreenXPosition() const final override;
		int getWantedScreenYPosition() const final override;
		float getFontSize() const final override;
		bool getFixNearFarPlanesWhenTranslatingFlag() const final override;
		bool getSelectedParticleShouldBeTopMost() const final override;
		bool getSelectedParticleForceShouldBeTopMost() const final override;

		bool shouldDisplayHelp() const;

		const Storm::SceneConfig& getSceneConfig() const final override;
		const Storm::SceneGraphicConfig& getSceneGraphicConfig() const final override;
		const Storm::SceneSimulationConfig& getSceneSimulationConfig() const final override;
		const std::vector<Storm::SceneRigidBodyConfig>& getSceneRigidBodiesConfig() const final override;
		const Storm::SceneFluidConfig& getSceneFluidConfig() const final override;
		const Storm::SceneRecordConfig& getSceneRecordConfig() const final override;
		const std::vector<Storm::SceneBlowerConfig>& getSceneBlowersConfig() const final override;
		const std::vector<Storm::SceneConstraintConfig>& getSceneConstraintsConfig() const final override;
		const Storm::SceneRigidBodyConfig& getSceneRigidBodyConfig(unsigned int rbId) const final override;
		const Storm::SceneScriptConfig& getSceneScriptConfig() const final override;

		bool isInReplayMode() const noexcept final override;
		bool userCanModifyTimestep() const noexcept final override;
		const std::string& getSceneName() const final override;
		const std::string& getSimulationTypeName() const final override;
		unsigned int getCurrentPID() const final override;
		const std::string& getSceneConfigFilePath() const final override;
		const std::string& getScriptFilePath() const final override;

	private:
		// Members that could be extracted from Command line.
		std::string _sceneConfigFilePath;

		std::string _temporaryPath;
		std::wstring _defaultSceneConfigFolderPath;
		std::string _exePath;
		bool _allowPopup;

		bool _shouldDisplayHelp;

		bool _shouldRegenerateParticleCache;
		bool _withUI;
		Storm::ThreadPriority _userSetThreadPriority;

		std::string _stateFileToLoad;
		bool _loadPhysicsTime;
		bool _loadVelocities;
		bool _loadForces;

		// Computed
		std::string _sceneFileName;
		unsigned int _currentPID;

		// Configs
		Storm::MacroConfig _macroConfig;
		Storm::GeneralConfig _generalConfig;
		Storm::SceneConfigHolder _sceneConfigHolder;
	};
}
