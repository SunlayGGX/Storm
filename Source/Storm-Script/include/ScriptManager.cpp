#include "ScriptManager.h"

#include "SingletonHolder.h"
#include "IThreadManager.h"
#include "ITimeManager.h"
#include "IInputManager.h"
#include "IConfigManager.h"

#include "SceneScriptConfig.h"

#include "LuaScriptWrapper.h"

#include "ScriptFile.h"

#include "CommandParser.h"
#include "ScriptObject.h"

#include "ThreadHelper.h"

#include "ThreadEnumeration.h"
#include "ThreadFlaggerObject.h"
#include "ThreadingSafety.h"

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
	_scriptWrapper{ std::make_unique<Storm::ScriptManager::UsedScriptWrapper>() },
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
	
	const Storm::SceneScriptConfig &sceneScriptConfig = configMgr.getSceneScriptConfig();

	if (sceneScriptConfig._enabled)
	{
		// Read the initialization file.
		const std::string &initScriptFilePath = sceneScriptConfig._initScriptFiles._filePath;
		if (!initScriptFilePath.empty())
		{
			LOG_DEBUG << "We'll execute the custom scripted initialization defined in the file " << initScriptFilePath;
			Storm::ScriptFile initScriptFile{ initScriptFilePath, true };
			initScriptFile.update();
		}

		const Storm::ScriptFilePipeConfig &filePipeConfig = sceneScriptConfig._scriptFilePipe;
		const std::string &watchedScriptFilePath = filePipeConfig._filePath;
		if (!watchedScriptFilePath.empty())
		{
			_watchedScriptFile = std::make_unique<Storm::ScriptFile>(watchedScriptFilePath);
			_refreshTimeDuration = std::chrono::milliseconds{ filePipeConfig._refreshRateInMillisec };
		}
	}

	_scriptThread = std::thread{ [this, &singletonHolder]()
	{
		STORM_REGISTER_THREAD(ScriptThread);
		STORM_DECLARE_THIS_THREAD_IS << Storm::ThreadFlagEnum::ScriptingThread;

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

void Storm::ScriptManager::executeScript_ScriptThread(std::string &&script)
{
	assert(Storm::isScriptThread() && "This method should only be called inside the script thread!");

	try
	{
		std::vector<Storm::ScriptObject> scriptObjects = Storm::CommandParser::parse(std::move(script));

		for (const Storm::ScriptObject &scriptObj : scriptObjects)
		{
			const std::string &scriptBody = scriptObj.getScript();

			LOG_SCRIPT_LOGIC << scriptBody;

			try
			{
				std::string errorMsg;

				if (!_scriptWrapper->execute(scriptBody, errorMsg))
				{
					if (!errorMsg.empty())
					{
						LOG_ERROR << "Script execution failed! Error was :\n" << errorMsg;
					}
					else
					{
						LOG_ERROR << "Script execution failed with an unknown error!";
					}
				}
			}
			catch (const Storm::Exception &ex)
			{
				LOG_ERROR <<
					"Script execution failed!\n"
					"Error was :\n" << ex.what() << ".\n"
					"Stack trace was :\n" << ex.stackTrace()
					;
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
	}
	catch (const Storm::Exception &ex)
	{
		LOG_ERROR <<
			"Script command parsing failed at :\n" << ex.stackTrace() <<
			"\n\n\nHere the details :\n\n" << ex.what()
			;
	}
	catch (const std::exception &ex)
	{
		LOG_ERROR << "Script command parsing failed! Error was :\n" << ex.what();
	}
	catch (...)
	{
		LOG_ERROR << "Script command parsing failed with an unknown error!";
	}
}

void Storm::ScriptManager::execute(std::string script)
{
	if (!script.empty())
	{
		Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::ScriptThread, [this, scriptContent = Storm::FuncMovePass<std::string>{ std::move(script) }]() mutable
		{
			this->executeScript_ScriptThread(std::move(scriptContent._object));
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
