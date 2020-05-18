#include "ConfigManager.h"
#include "CommandLineParser.h"

#include "ThrowException.h"

#include <filesystem>


namespace
{
    void checkFileExistence(const std::string &pathStr)
    {
        if (!std::filesystem::is_regular_file(pathStr))
        {
            Storm::throwException<std::exception>(pathStr + " should be pointing to a correct file.");
        }
    }
}


Storm::ConfigManager::ConfigManager() : 
    _shouldDisplayHelp{ false }
{

}

Storm::ConfigManager::~ConfigManager() = default;


void Storm::ConfigManager::initialize_Implementation(int argc, const char* argv[])
{
    LOG_DEBUG << "Reading Config file started";
    
    Storm::CommandLineParser parser{ argc, argv };
    _shouldDisplayHelp = parser.shouldDisplayHelp();
    if (!_shouldDisplayHelp)
    {
        _sceneConfigFilePath = parser.getSceneFilePath();
        checkFileExistence(_sceneConfigFilePath);

        _inputBindingsConfigFilePath = parser.getInputConfigFilePath();
        checkFileExistence(_inputBindingsConfigFilePath);

        _temporaryPath = parser.getTempPath();
        std::filesystem::path tempPath{ _temporaryPath };
        if (std::filesystem::exists(tempPath))
        {
            if (!std::filesystem::is_directory(_temporaryPath))
            {
                Storm::throwException<std::exception>(_temporaryPath + " should either be a folder, or shouldn't exists!");
            }
        }
        else
        {
            std::filesystem::create_directories(_temporaryPath);
        }
    }
    else
    {
        LOG_COMMENT << "Command line help Requested:\n" << parser.getHelp();
    }

    LOG_DEBUG << "Reading Config file ended";
}

const std::string& Storm::ConfigManager::getTemporaryPath() const
{
    return _temporaryPath;
}

const std::string& Storm::ConfigManager::getLogFileName() const
{
    return _logFileName;
}

bool Storm::ConfigManager::noPopup() const
{
    return _allowPopup;
}

bool Storm::ConfigManager::shouldDisplayHelp() const
{
    return _shouldDisplayHelp;
}
