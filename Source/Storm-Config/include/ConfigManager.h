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

		const std::string& getLogFolderPath() const final override;
		Storm::LogLevel getLogLevel() const final override;
		int getRemoveLogOlderThanDaysCount() const final override;
		bool getShouldOverrideOldLog() const final override;
		bool getShouldLogFpsWatching() const final override;
		bool noPopup() const final override;

		bool shouldDisplayHelp() const;

		virtual const Storm::SceneData& getSceneData() const override;
		virtual const Storm::GraphicData& getGraphicData() const override;
		virtual const std::vector<Storm::RigidBodySceneData>& getRigidBodiesData() const override;

	private:
		// Members that could be extracted from Command line.
		std::string _sceneConfigFilePath;

		std::string _temporaryPath;
		std::wstring _defaultSceneConfigFolderPath;
		std::string _exePath;
		bool _allowPopup;

		bool _shouldDisplayHelp;

		// Configs
		Storm::MacroConfig _macroConfig;
		Storm::GeneralConfig _generalConfig;
		Storm::SceneConfig _sceneConfig;
	};
}
