#pragma once

#include "Singleton.h"
#include "IConfigManager.h"
#include "SingletonDefaultImplementation.h"

#include "MacroConfigHolder.h"
#include "GeneralConfigHolder.h"
#include "SceneConfigHolder.h"
#include "InternalConfigHolder.h"


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
		const std::string& getExePath() const final override;

		bool shouldRegenerateParticleCache() const final override;

		bool withUI() const final override;

		Storm::ThreadPriority getUserSetThreadPriority() const final override;

		const std::string& getStateFilePath() const final override;
		void stateShouldLoad(bool &outLoadPhysicsTime, bool &outLoadForces, bool &outLoadVelocities) const final override;

		bool clearAllLogs() const final override;

		bool shouldDisplayHelp() const;

		const Storm::GeneratedGitConfig& getInternalGeneratedGitConfig() const final override;
		const std::vector<Storm::InternalReferenceConfig>& getInternalReferencesConfig() const final override;

		const Storm::GeneralGraphicConfig& getGeneralGraphicConfig() const final override;
		const Storm::GeneralSimulationConfig& getGeneralSimulationConfig() const final override;
		const Storm::GeneralNetworkConfig& getGeneralNetworkConfig() const final override;
		const Storm::GeneralWebConfig& getGeneralWebConfig() const final override;
		const Storm::GeneralDebugConfig& getGeneralDebugConfig() const final override;
		const Storm::GeneralApplicationConfig& getGeneralApplicationConfig() const final override;
		const Storm::GeneralSafetyConfig& getGeneralSafetyConfig() const final override;
		const Storm::GeneralArchiveConfig& getGeneralArchiveConfig() const final override;

		const Storm::SceneGraphicConfig& getSceneGraphicConfig() const final override;
		const Storm::SceneSimulationConfig& getSceneSimulationConfig() const final override;
		const Storm::ScenePhysicsConfig& getScenePhysicsConfig() const final override;
		const std::vector<Storm::SceneRigidBodyConfig>& getSceneRigidBodiesConfig() const final override;
		const Storm::SceneFluidConfig& getSceneFluidConfig() const final override;
		const Storm::SceneRecordConfig& getSceneRecordConfig() const final override;
		const std::vector<Storm::SceneBlowerConfig>& getSceneBlowersConfig() const final override;
		const std::vector<Storm::SceneConstraintConfig>& getSceneConstraintsConfig() const final override;
		const Storm::SceneRigidBodyConfig& getSceneRigidBodyConfig(unsigned int rbId) const final override;
		const Storm::SceneScriptConfig& getSceneScriptConfig() const final override;

		const Storm::SceneCageConfig* getSceneOptionalCageConfig() const final override;

		bool isInReplayMode() const noexcept final override;
		bool isInRecordMode() const noexcept final override;
		bool userCanModifyTimestep() const noexcept final override;
		const std::string& getSceneName() const final override;
		const std::string& getSimulationTypeName() const final override;
		std::string getViscosityMethods() const final override;
		unsigned int getCurrentPID() const final override;
		const std::string& getComputerName() const final override;
		const std::string& getSceneConfigFilePath() const final override;
		const std::string& getScriptFilePath() const final override;
		const std::string& getRestartCommandline() const final override;
		bool hasWall() const final override;

		void getUnsafeMacroizedConvertedValue(std::string &inOutValue) const final override;
		bool getMaybeMacroizedConvertedValue(std::string &inOutValue) const final override;
		void getMacroizedConvertedValue(std::string &inOutValue) const final override;
		std::string makeMacroKey(const std::string_view value) const final override;

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

		bool _clearLogs;

		// Computed
		std::string _sceneFileName;
		unsigned int _currentPID;
		std::string _computerName;

		std::string _commandLineForRestart;

		// Configs
		Storm::MacroConfigHolder _macroConfig;
		Storm::GeneralConfigHolder _generalConfigHolder;
		Storm::SceneConfigHolder _sceneConfigHolder;
		Storm::InternalConfigHolder _internalConfigHolder;
	};
}
