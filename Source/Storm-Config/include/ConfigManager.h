#pragma once

#include "Singleton.h"
#include "IConfigManager.h"
#include "SingletonDefaultImplementation.h"

#include "MacroConfig.h"
#include "GeneralConfig.h"
#include "SceneConfig.h"


namespace Storm
{
	class ConfigManager :
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

		const std::string& getLogFolderPath() const final override;
		Storm::LogLevel getLogLevel() const final override;
		int getRemoveLogOlderThanDaysCount() const final override;
		bool getShouldOverrideOldLog() const final override;
		bool getShouldLogFpsWatching() const final override;
		bool noPopup() const final override;

		unsigned int getWantedScreenWidth() const final override;
		unsigned int getWantedScreenHeight() const final override;

		bool shouldDisplayHelp() const;

		const std::string& getSceneName() const final override;
		const Storm::SceneData& getSceneData() const final override;
		const Storm::GraphicData& getGraphicData() const final override;
		const Storm::GeneralSimulationData& getGeneralSimulationData() const final override;
		const std::vector<Storm::RigidBodySceneData>& getRigidBodiesData() const final override;
		const Storm::FluidData& getFluidData() const final override;
		const Storm::RigidBodySceneData& getRigidBodyData(unsigned int rbId) const final override = 0;

	private:
		// Members that could be extracted from Command line.
		std::string _sceneConfigFilePath;

		std::string _temporaryPath;
		std::wstring _defaultSceneConfigFolderPath;
		std::string _exePath;
		bool _allowPopup;

		bool _shouldDisplayHelp;

		bool _shouldRegenerateParticleCache;

		// Computed
		std::string _sceneFileName;

		// Configs
		Storm::MacroConfig _macroConfig;
		Storm::GeneralConfig _generalConfig;
		Storm::SceneConfig _sceneConfig;
	};
}
