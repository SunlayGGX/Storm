#include "StormProcessOpener.h"

#include "SingletonHolder.h"
#include "ISimulatorManager.h"
#include "IConfigManager.h"

#include "OSManager.h"

#include "StormProcessStartup.h"

#include "SceneScriptConfig.h"

#include "ExitCode.h"

#include "StringAlgo.h"


namespace
{
	enum : std::size_t { k_failureIndex = std::numeric_limits<std::size_t>::max() };

	bool tryOpenProcess(const Storm::StormProcessOpener::OpenParameter &param, std::size_t &outProcessUID, Storm::StormProcessStartup &&startUpProcess)
	{
		Storm::OSManager &osMgr = Storm::OSManager::instance();

		const std::string processName = Storm::toStdString(std::filesystem::path{ startUpProcess._exePath }.stem());

		LOG_DEBUG << "We'll try to open " << processName;

		try
		{
			outProcessUID = osMgr.startProcess(std::move(startUpProcess));
			return true;
		}
		catch (const Storm::StormException &ex)
		{
			LOG_ERROR <<
				"Failed to open " << processName << ".\n"
				"Error : " << ex.what() << "\n\n"
				"Stack trace :\n" << ex.stackTrace()
				;
		}
		catch (const std::exception &ex)
		{
			LOG_ERROR <<
				"Failed to open " << processName << ".\n"
				"Error (std::exception) : " << ex.what()
				;
		}
		catch (...)
		{
			LOG_ERROR << "Failed to open " << processName << " due to an unknown error.";
		}

		if (param._failureQuit)
		{
			Storm::ISimulatorManager &simulMgr = Storm::SingletonHolder::instance().getSingleton<Storm::ISimulatorManager>();
			simulMgr.exitWithCode(Storm::ExitCode::k_otherThreadTermination);
		}

		outProcessUID = k_failureIndex;
		return false;
	}

	bool openNotepadOnFile(const Storm::StormProcessOpener::OpenParameter &param, std::size_t &outProcessUID, const std::string &onFilePath)
	{
		if (!tryOpenProcess(param, outProcessUID, Storm::StormProcessStartup{
				._commandLine = "start notepad++ \"" + onFilePath + '"',
				._bindIO = false,
				._shareLife = false,
				._isCmd = true
			}))
		{
			// If we don't have notepad++ installed, just start the old, weird, ugly, unfit to work with... Windows embedded notepad.
			return tryOpenProcess(param, outProcessUID, Storm::StormProcessStartup{
				._commandLine = "notepad \"" + onFilePath + '"',
				._bindIO = false,
				._shareLife = false,
				._isCmd = true
			});
		}

		return true;
	}

	template<class StringType>
	std::string mountURLCommand(const StringType &url)
	{
		std::string result;
		result.reserve(Storm::StringAlgo::extractSize(url) + 64);

		const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();

		result += "start chrome.exe ";

		/*if (configMgr.urlOpenIncognito())
		{
			result += "-incognito ";
		}*/

		result += "--new-window ";

		result += url;

		return result;
	}
}


bool Storm::StormProcessOpener::openStormLogViewer(const Storm::StormProcessOpener::OpenParameter &param, std::size_t &outProcessUID)
{
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();

	const std::filesystem::path exeFolderPath = std::filesystem::path{ configMgr.getExePath() }.parent_path();

	return tryOpenProcess(param, outProcessUID, Storm::StormProcessStartup{
		._exePath = (exeFolderPath / STORM_EXECUTABLE_NAME("Storm-LogViewer")).string(),
		._workingDirectoryPath = exeFolderPath.string(),
		._commandLine = "",
		._bindIO = true,
		._shareLife = true,
		._isCmd = false
	});
}

bool Storm::StormProcessOpener::openRuntimeScript(const Storm::StormProcessOpener::OpenParameter &param, std::size_t &outProcessUID)
{
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const std::string &scriptPath = configMgr.getSceneScriptConfig()._scriptFilePipe._filePath;

	return openNotepadOnFile(param, outProcessUID, scriptPath);
}

bool Storm::StormProcessOpener::openCurrentConfigFile(const Storm::StormProcessOpener::OpenParameter &param, std::size_t &outProcessUID)
{
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const std::string &sceneConfigFilePath = configMgr.getSceneConfigFilePath();

	return openNotepadOnFile(param, outProcessUID, sceneConfigFilePath);
}

bool Storm::StormProcessOpener::openStormGithubLink(const OpenParameter &param, std::size_t &outProcessUID)
{
	return tryOpenProcess(param, outProcessUID, Storm::StormProcessStartup{
		._commandLine = mountURLCommand("https://github.com/SunlayGGX/Storm"),
		._bindIO = false,
		._shareLife = false,
		._isCmd = true
	});
}
