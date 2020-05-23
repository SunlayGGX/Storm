#pragma once
#include "Singleton.h"
#include "IConfigManager.h"
#include "SingletonDefaultImplementation.h"


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
        bool noPopup() const final override;

        bool shouldDisplayHelp() const;

    private:
        std::string _sceneConfigFilePath;

        std::string _temporaryPath;
        std::string _logFileName;
        std::wstring _defaultConfigFolderPath;
        std::string _exePath;
        bool _allowPopup;

        bool _shouldDisplayHelp;
    };
}
