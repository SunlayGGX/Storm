#include "ScriptManager.h"

#include "SingletonHolder.h"
#include "IThreadManager.h"
#include "ITimeManager.h"
#include "IInputManager.h"

#include "ScriptFile.h"

#include "ThreadHelper.h"

#include "ThreadEnumeration.h"
#include "SpecialKey.h"


Storm::ScriptManager::ScriptManager() = default;
Storm::ScriptManager::~ScriptManager() = default;

void Storm::ScriptManager::initialize_Implementation()
{
	LOG_COMMENT << "Initializing scripting manager";

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

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

	_watchedScriptFiles.clear();
}

void Storm::ScriptManager::executeScript(const std::string &script)
{
	LOG_DEBUG << script;

	try
	{
		// TODO
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
		Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(std::this_thread::get_id(), [this, scriptContent = std::move(script)]()
		{
			this->executeScript(scriptContent);
		});
	}
}

void Storm::ScriptManager::rereadWatchedScriptFile()
{
	for (Storm::ScriptFile &watchedFile : _watchedScriptFiles)
	{
		watchedFile.invalidate();
		watchedFile.update();
	}
}

void Storm::ScriptManager::refreshWatchedScriptFile()
{
	for (Storm::ScriptFile &watchedFile : _watchedScriptFiles)
	{
		watchedFile.update();
	}
}

void Storm::ScriptManager::execute(const Storm::ThreadEnumeration threadEnum, std::string script)
{
	if (!script.empty())
	{
		Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(threadEnum, [this, scriptContent = std::move(script)]()
		{
			this->executeScript(scriptContent);
		});
	}
}
