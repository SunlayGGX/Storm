#include "ScriptManager.h"

#include "SingletonHolder.h"
#include "IThreadManager.h"
#include "ITimeManager.h"
#include "IInputManager.h"
#include "IConfigManager.h"

#include "ScriptData.h"

#include "LuaScriptWrapper.h"

#include "ScriptFile.h"

#include "ThreadHelper.h"

#include "ThreadEnumeration.h"
#include "SpecialKey.h"

#include "FuncMovePass.h"


namespace
{
	void invalidateWatchedScriptFile(Storm::ScriptFile &watchedFile)
	{
		watchedFile.invalidate();
		watchedFile.update();
	}

	void updateWatchedScriptFile(Storm::ScriptFile &watchedFile)
	{
		watchedFile.update();
	}
}


Storm::ScriptManager::ScriptManager() :
	_scriptWrapper{ std::make_unique<Storm::IScriptManager::UsedScriptWrapper>() },
	_nextWatchedScriptFileUpdate{ std::chrono::high_resolution_clock::now() },
	_refreshTimeDuration{ 0 }
{

}

Storm::ScriptManager::~ScriptManager() = default;

void Storm::ScriptManager::initialize_Implementation()
{
	LOG_COMMENT << "Initializing scripting manager";

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
	
	const Storm::ScriptData &scriptConfigData = configMgr.getScriptData();

	if (scriptConfigData._enabled)
	{
		// Read the initialization file.
		const std::string &initScriptFilePath = scriptConfigData._initScriptFiles._filePath;
		if (!initScriptFilePath.empty())
		{
			LOG_DEBUG << "We'll execute the custom scripted initialization defined in the file " << initScriptFilePath;
			Storm::ScriptFile initScriptFile{ initScriptFilePath, true };
			initScriptFile.update();
		}

		const Storm::ScriptFilePipeData &filePipe = scriptConfigData._scriptFilePipe;
		const std::string &watchedScriptFilePath = filePipe._filePath;
		if (!watchedScriptFilePath.empty())
		{
			_watchedScriptFile = std::make_unique<Storm::ScriptFile>(watchedScriptFilePath);
			_refreshTimeDuration = std::chrono::milliseconds{ filePipe._refreshRateInMillisec };
		}
	}

	_scriptThread = std::thread{ [this, &singletonHolder]()
	{
		STORM_REGISTER_THREAD(ScriptThread);

		{
			Storm::IInputManager &inputMgr = singletonHolder.getSingleton<Storm::IInputManager>();
			inputMgr.bindKey(Storm::SpecialKey::KC_J, [this]() { this->rereadWatchedScriptFile(); });
		}

		Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();
		Storm::IThreadManager &threadMgr = singletonHolder.getSingleton<Storm::IThreadManager>();

		while (timeMgr.waitNextFrameOrExit())
		{
			threadMgr.processCurrentThreadActions();
			this->refreshWatchedScriptFile();
		}
	} };
}

void Storm::ScriptManager::cleanUp_Implementation()
{
	LOG_COMMENT << "Cleaning up scripting manager";

	Storm::join(_scriptThread);

	_scriptWrapper.reset();
}

void Storm::ScriptManager::executeScript(const std::string &script)
{
	LOG_DEBUG << script;

	try
	{
		_scriptWrapper->execute(script);
	}
	catch (const std::exception &ex)
	{
		LOG_ERROR << "Script execution failed! Error was :\n" << ex.what();
	}
	catch (...)
	{
		LOG_ERROR << "Script execution failed with an unknown error!";
	}
}

void Storm::ScriptManager::execute(std::string script)
{
	if (!script.empty())
	{
		Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::ScriptThread, [this, scriptContent = Storm::FuncMovePass<std::string>{ std::move(script) }]()
		{
			this->executeScript(scriptContent._object);
		});
	}
}

Storm::ScriptManager::UsedScriptWrapper& Storm::ScriptManager::getScriptWrapper()
{
	return *_scriptWrapper;
}

void Storm::ScriptManager::rereadWatchedScriptFile()
{
	if (_watchedScriptFile)
	{
		invalidateWatchedScriptFile(*_watchedScriptFile);
	}
}

void Storm::ScriptManager::refreshWatchedScriptFile()
{
	// Refresh the script once every 4 frames.
	if (_watchedScriptFile && std::chrono::high_resolution_clock::now() > _nextWatchedScriptFileUpdate)
	{
		updateWatchedScriptFile(*_watchedScriptFile);
		_nextWatchedScriptFileUpdate = std::chrono::high_resolution_clock::now() + _refreshTimeDuration;
	}
}
