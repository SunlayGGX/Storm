#include "StormProcessOpener.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"
#include "IWebManager.h"

#include "OSManager.h"

#include "StormProcessStartup.h"

#include "SceneScriptConfig.h"

#include "StormExiter.h"

#include "StringAlgo.h"


namespace
{
	enum : std::size_t { k_failureIndex = std::numeric_limits<std::size_t>::max() };

	std::size_t defaultOpenProcess(std::string &outProcessName, Storm::StormProcessStartup &&startUpProcess)
	{
		outProcessName = Storm::toStdString(std::filesystem::path{ startUpProcess._exePath }.stem());

		LOG_DEBUG << "We'll try to open " << outProcessName;

		Storm::OSManager &osMgr = Storm::OSManager::instance();
		return osMgr.startProcess(std::move(startUpProcess));
	}

	std::size_t defaultOpenURL(std::string &outURL, const std::string_view &url)
	{
		outURL = url;

		LOG_DEBUG << "We'll try to open url " << outURL;

		Storm::IWebManager &webMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IWebManager>();
		return webMgr.openURL(url);
	}

	template<class ExecutorFunc, class ... Args>
	bool tryOpenProcess(const Storm::StormProcessOpener::OpenParameter &param, std::size_t &outProcessUID, const ExecutorFunc &execFunc, Args &&...args)
	{
		std::string nameOfWhatToOpen;

		try
		{
			outProcessUID = execFunc(nameOfWhatToOpen, std::forward<Args>(args)...);
			return outProcessUID != k_failureIndex;
		}
		catch (const Storm::Exception &ex)
		{
			LOG_ERROR <<
				"Failed to open " << nameOfWhatToOpen << ".\n"
				"Error : " << ex.what() << "\n\n"
				"Stack trace :\n" << ex.stackTrace()
				;
		}
		catch (const std::exception &ex)
		{
			LOG_ERROR <<
				"Failed to open " << nameOfWhatToOpen << ".\n"
				"Error (std::exception) : " << ex.what()
				;
		}
		catch (...)
		{
			LOG_ERROR << "Failed to open " << nameOfWhatToOpen << " due to an unknown error.";
		}

		if (param._failureQuit)
		{
			Storm::requestExitOtherThread();
		}

		outProcessUID = k_failureIndex;
		return false;
	}

	bool openNotepadOnFile(const Storm::StormProcessOpener::OpenParameter &param, std::size_t &outProcessUID, const std::string &onFilePath)
	{
		if (!tryOpenProcess(param, outProcessUID, defaultOpenProcess, Storm::StormProcessStartup{
				._commandLine = "start notepad++ \"" + onFilePath + '"',
				._bindIO = false,
				._shareLife = false,
				._isCmd = true
			}))
		{
			// If we don't have notepad++ installed, just start the old, weird, ugly, unfit to work with... Windows embedded notepad.
			return tryOpenProcess(param, outProcessUID, defaultOpenProcess, Storm::StormProcessStartup{
				._commandLine = "notepad \"" + onFilePath + '"',
				._bindIO = false,
				._shareLife = false,
				._isCmd = true
			});
		}

		return true;
	}
}


bool Storm::StormProcessOpener::openStormLogViewer(const Storm::StormProcessOpener::OpenParameter &param, std::size_t &outProcessUID)
{
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();

	const std::filesystem::path exeFolderPath = std::filesystem::path{ configMgr.getExePath() }.parent_path();

	return tryOpenProcess(param, outProcessUID, defaultOpenProcess, Storm::StormProcessStartup{
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

bool Storm::StormProcessOpener::openTextFile(const OpenParameter &param, std::size_t &outProcessUID)
{
	return openNotepadOnFile(param, outProcessUID, std::string{ param._additionalParameterStr });
}

bool Storm::StormProcessOpener::openStormUrlLink(const Storm::StormProcessOpener::OpenParameter &param, std::size_t &outProcessUID)
{
	return tryOpenProcess(param, outProcessUID, defaultOpenURL, param._additionalParameterStr);
}

bool Storm::StormProcessOpener::openStormRestarter(const Storm::StormProcessOpener::OpenParameter &param, std::size_t &outProcessUID)
{
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();

	const std::filesystem::path exeFolderPath = std::filesystem::path{ configMgr.getExePath() }.parent_path();

	const std::string stormRestarterPath = STORM_EXECUTABLE_NAME("Storm-Restarter");
	const std::string &restartCommandline = configMgr.getRestartCommandline();

	const std::size_t additionalArgsSize = param._additionalParameterStr.size();

	std::string restartCommandBuild;
	restartCommandBuild.reserve(stormRestarterPath.size() + restartCommandline.size() + additionalArgsSize + 12);

	restartCommandBuild += "start ";
	restartCommandBuild += stormRestarterPath;
	restartCommandBuild += ' ';
	restartCommandBuild += restartCommandline;
	if (additionalArgsSize > 0)
	{
		restartCommandBuild += ' ';
		restartCommandBuild += param._additionalParameterStr;
	}

	Storm::StringAlgo::replaceAll(restartCommandBuild, "\\\"", Storm::StringAlgo::makeReplacePredicate('"'));

	return tryOpenProcess(param, outProcessUID, defaultOpenProcess, Storm::StormProcessStartup{
		._workingDirectoryPath = exeFolderPath.string(),
		._commandLine = std::move(restartCommandBuild),
		._bindIO = false,
		._shareLife = false,
		._isCmd = true
	});
}

bool Storm::StormProcessOpener::openStormRootExplorer(const Storm::StormProcessOpener::OpenParameter &param, std::size_t &outProcessUID)
{
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();

	std::string openCommand = "Explorer.exe \"$[StormRoot]\"";
	configMgr.getMacroizedConvertedValue(openCommand);

	return tryOpenProcess(param, outProcessUID, defaultOpenProcess, Storm::StormProcessStartup{
		._commandLine = std::move(openCommand),
		._bindIO = false,
		._shareLife = false,
		._isCmd = true
	});
}
