#include "ConfigManager.h"
#include "CommandLineParser.h"

#include "ThrowException.h"

#include "SingletonHolder.h"
#include "IOSManager.h"

#include <boost/algorithm/string.hpp>


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
		_exePath = argv[0];

		_macroConfig.initialize();

		std::filesystem::path configFolderPath{ *_macroConfig.queryMacroValue("StormConfig") };
		std::filesystem::path customConfigFolderPath = configFolderPath / "Custom";
		std::filesystem::path defaultSceneConfigFolderPath = customConfigFolderPath / "Scenes";
		std::filesystem::path defaultGeneralConfigFolderPath = customConfigFolderPath / "General";

		if (std::filesystem::exists(defaultSceneConfigFolderPath))
		{
			_defaultSceneConfigFolderPath = defaultSceneConfigFolderPath.wstring();
		}
		else
		{
			LOG_WARNING << "Storm file tree order isn't standard. Cannot grab the default config folder path... Therefore it will remain empty.";
		}

		const std::filesystem::path k_macroConfigFileName{ "Macro.xml" };
		std::filesystem::path defaultMacroConfigFolderPath = defaultGeneralConfigFolderPath / k_macroConfigFileName;
		if (!std::filesystem::exists(defaultMacroConfigFolderPath))
		{
			defaultMacroConfigFolderPath = defaultGeneralConfigFolderPath / "Original" / k_macroConfigFileName;
		}

		std::filesystem::path macroConfigFolderPath;
		// Here we would just use the built-in macros since we did not read the config yet. But it can be useful ;)
		const std::string macroConfigFilePathStrFromCmdLine = _macroConfig(parser.getMacroConfigFilePath());
		if (macroConfigFilePathStrFromCmdLine.empty() || !_macroConfig.read(macroConfigFilePathStrFromCmdLine))
		{
			_macroConfig.read(defaultMacroConfigFolderPath.string());
		}

		_sceneConfigFilePath = _macroConfig(parser.getSceneFilePath());
		if (_sceneConfigFilePath.empty())
		{
			LOG_COMMENT << "No scene file passed from command line. Will ask user the config file with an explorer file dialog.";

			const std::map<std::wstring, std::wstring> fileFilters{
				{ L"Xml file (*.xml)", L"*.xml" }
			};

			_sceneConfigFilePath = std::filesystem::path{ Storm::SingletonHolder::instance().getFacet<Storm::IOSManager>()->openFileExplorerDialog(_defaultSceneConfigFolderPath, fileFilters) }.string();
		}

		std::string errorMsg;

		const std::filesystem::path sceneConfigFilePath{ _sceneConfigFilePath };
		if (_sceneConfigFilePath.empty())
		{
			errorMsg = "No scenes config file set. The application cannot run thus we will abort execution!";
		}
		else if (!std::filesystem::is_regular_file(sceneConfigFilePath))
		{
			errorMsg = _sceneConfigFilePath + " should be pointing to a correct file.";
		}
		else if (boost::algorithm::to_lower_copy(sceneConfigFilePath.extension().string()) != ".xml")
		{
			errorMsg = _sceneConfigFilePath + " should be pointing to an xml file!";
		}

		if (!errorMsg.empty())
		{
			LOG_FATAL << errorMsg;
			Storm::throwException<std::exception>(errorMsg);
		}

		_temporaryPath = _macroConfig(parser.getTempPath());
		std::filesystem::path tempPath{ _temporaryPath };
		if (std::filesystem::exists(tempPath))
		{
			if (!std::filesystem::is_directory(_temporaryPath))
			{
				errorMsg = _temporaryPath + " should either be a folder, or shouldn't exists!";
			}
		}
		else
		{
			std::filesystem::create_directories(_temporaryPath);
		}

		if (!errorMsg.empty())
		{
			LOG_FATAL << errorMsg;
			Storm::throwException<std::exception>(errorMsg);
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

const std::string& Storm::ConfigManager::getExePath() const
{
	return _exePath;
}

bool Storm::ConfigManager::noPopup() const
{
	return _allowPopup;
}

bool Storm::ConfigManager::shouldDisplayHelp() const
{
	return _shouldDisplayHelp;
}
