#include "ConfigManager.h"


Storm::ConfigManager::ConfigManager() = default;
Storm::ConfigManager::~ConfigManager() = default;


void Storm::ConfigManager::initialize_Implementation(int argc, const char* argv[])
{
    // TODO
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
