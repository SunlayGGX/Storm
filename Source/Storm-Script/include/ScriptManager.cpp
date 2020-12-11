#include "ScriptManager.h"


Storm::ScriptManager::ScriptManager() = default;
Storm::ScriptManager::~ScriptManager() = default;

void Storm::ScriptManager::initialize_Implementation()
{
	LOG_COMMENT << "Initializing scripting manager";
}

void Storm::ScriptManager::cleanUp_Implementation()
{
	LOG_COMMENT << "Cleaning up scripting manager";
}
