#include "ConfigManager.h"
#include "CommandLineParser.h"

#include "ThrowException.h"

#include "SingletonHolder.h"
#include "IOSManager.h"

#include "SceneData.h"
#include "RigidBodySceneData.h"
#include "RecordConfigData.h"

#include "RecordMode.h"

#include <boost/algorithm/string.hpp>


Storm::ConfigManager::ConfigManager() :
	_shouldDisplayHelp{ false },
	_shouldRegenerateParticleCache{ false }
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
		const std::filesystem::path exePath{ _exePath };

		// If is not absolute, make it absolute.
		bool hasMadeAbsolute = !exePath.is_absolute();
		if (hasMadeAbsolute)
		{
			_exePath = (std::filesystem::current_path() / exePath.filename()).string();
		}

		_macroConfig.initialize();

		// Set the current working directory path to the path of the executable to prevent mismatch depending on where we start the application.
		// This can happen for example if we start the exe from VS, or start it from cmd line : We would like the same behavior in both cases.
		if (!hasMadeAbsolute)
		{
			const std::filesystem::path exeFolderPath = _macroConfig("$[StormFolderExe]");
			std::filesystem::current_path(exeFolderPath);
		}

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
		std::filesystem::path defaultMacroConfigFilePath = defaultGeneralConfigFolderPath / k_macroConfigFileName;
		if (!std::filesystem::exists(defaultMacroConfigFilePath))
		{
			defaultMacroConfigFilePath = defaultGeneralConfigFolderPath / "Original" / k_macroConfigFileName;
		}

		const std::filesystem::path k_generalConfigFileName{ "Global.xml" };
		std::filesystem::path defaultGeneralConfigFilePath = defaultGeneralConfigFolderPath / k_generalConfigFileName;
		if (!std::filesystem::exists(defaultGeneralConfigFilePath))
		{
			defaultGeneralConfigFilePath = defaultGeneralConfigFolderPath / "Original" / k_generalConfigFileName;
		}

		std::filesystem::path macroConfigFolderPath;
		// Here we would just use the built-in macros since we did not read the config yet. But it can be useful ;)
		const std::string macroConfigFilePathStrFromCmdLine = _macroConfig(parser.getMacroConfigFilePath());
		if (macroConfigFilePathStrFromCmdLine.empty() || !_macroConfig.read(macroConfigFilePathStrFromCmdLine))
		{
			_macroConfig.read(defaultMacroConfigFilePath.string());
		}

		const std::string generalConfigFilePathStrFromCmdLine = _macroConfig(parser.getGeneralConfigFilePath());
		if (generalConfigFilePathStrFromCmdLine.empty() || !_generalConfig.read(generalConfigFilePathStrFromCmdLine))
		{
			_generalConfig.read(defaultGeneralConfigFilePath.string());
		}

		_generalConfig.applyMacros(_macroConfig);

		_sceneConfigFilePath = _macroConfig(parser.getSceneFilePath());
		if (_sceneConfigFilePath.empty())
		{
			LOG_COMMENT << "No scene file passed from command line. Will ask user the config file with an explorer file dialog.";

			const std::map<std::wstring, std::wstring> fileFilters{
				{ L"Xml file (*.xml)", L"*.xml" }
			};

			_sceneConfigFilePath = Storm::toStdString(Storm::SingletonHolder::instance().getSingleton<Storm::IOSManager>().openFileExplorerDialog(_defaultSceneConfigFolderPath, fileFilters));
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

		if (errorMsg.empty())
		{
			_sceneFileName = sceneConfigFilePath.stem().string();
			_sceneConfig.read(_sceneConfigFilePath, _macroConfig, _generalConfig);
		}
		else
		{
			LOG_FATAL << errorMsg;
			Storm::throwException<std::exception>(errorMsg);
		}

		_temporaryPath = _macroConfig(parser.getTempPath());
		std::filesystem::path tempPath{ _temporaryPath };
		if (std::filesystem::exists(tempPath))
		{
			if (!std::filesystem::is_directory(tempPath))
			{
				errorMsg = _temporaryPath + " should either be a folder, or shouldn't exists!";
			}
		}
		else if (_temporaryPath.empty())
		{
			_temporaryPath = _macroConfig("$[StormTmp]");
		}

		std::filesystem::create_directories(_temporaryPath);

		if (!errorMsg.empty())
		{
			LOG_FATAL << errorMsg;
			Storm::throwException<std::exception>(errorMsg);
		}

		_shouldRegenerateParticleCache = parser.getShouldRegenerateParticleCache();

		std::string recordModeStr = parser.getRecordModeStr();
		boost::algorithm::to_lower(recordModeStr);

		Storm::RecordConfigData &recordConfigData = *_sceneConfig.getSceneData()._recordConfigData;
		recordConfigData._recordMode =
			recordModeStr == "record" ? Storm::RecordMode::Record :
			recordModeStr == "replay" ? Storm::RecordMode::Replay :
			Storm::RecordMode::None;

		if (!recordModeStr.empty() && recordConfigData._recordMode != Storm::RecordMode::None)
		{
			switch (recordConfigData._recordMode)
			{
			case Storm::RecordMode::None:
			case Storm::RecordMode::Record:
				break;

			case Storm::RecordMode::Replay:
				Storm::throwException<std::exception>("Replay mode is not implemented yet");

			default:
				Storm::throwException<std::exception>("Unknown record mode command line tag \"" + recordModeStr + '"');
			}
		}

		std::string recordFilePath;
		switch (recordConfigData._recordMode)
		{
		case Storm::RecordMode::Record:
			recordFilePath = parser.getRecordFilePath();
			if (!recordFilePath.empty())
			{
				recordConfigData._recordFilePath = std::move(recordFilePath);
			}
			_macroConfig(recordConfigData._recordFilePath);
			if (recordConfigData._recordFilePath.empty())
			{
				Storm::throwException<std::exception>("Record file path should be set when in record mode!");
			}
			else if (recordConfigData._recordFps == -1.f) // Check manually set invalid values was done before. Here we check the unset.
			{
				Storm::throwException<std::exception>("Record fps wasn't set while we should be recording. We should always set one!");
			}
			std::filesystem::remove_all(recordConfigData._recordFilePath);
			break;

		case Storm::RecordMode::Replay:
			recordFilePath = parser.getRecordFilePath();
			if (!recordFilePath.empty())
			{
				recordConfigData._recordFilePath = std::move(recordFilePath);
			}
			_macroConfig(recordConfigData._recordFilePath);
			if (!std::filesystem::is_regular_file(recordConfigData._recordFilePath))
			{
				Storm::throwException<std::exception>(recordConfigData._recordFilePath + " doesn't exist or isn't a regular record file!");
			}
			break;

		case Storm::RecordMode::None:
		default:
			recordFilePath = parser.getRecordFilePath();
			if (!recordFilePath.empty())
			{
				Storm::throwException<std::exception>("A record file from command line has been set (" + recordFilePath + "). But we're not recording or replaying, it is forbidden!");
			}
			// No need to check the one coming from the scene config file because it is just a placeholder in case the command line one isn't set.
			break;
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

const std::string& Storm::ConfigManager::getExePath() const
{
	return _exePath;
}

bool Storm::ConfigManager::shouldRegenerateParticleCache() const
{
	return _shouldRegenerateParticleCache;
}

bool Storm::ConfigManager::noPopup() const
{
	return _allowPopup;
}

bool Storm::ConfigManager::getShouldProfileSimulationSpeed() const
{
	return _generalConfig._profileSimulationSpeed;
}

unsigned int Storm::ConfigManager::getWantedScreenWidth() const
{
	return _generalConfig._wantedApplicationWidth;
}

unsigned int Storm::ConfigManager::getWantedScreenHeight() const
{
	return _generalConfig._wantedApplicationHeight;
}

float Storm::ConfigManager::getFontSize() const
{
	return _generalConfig._fontSize;
}

bool Storm::ConfigManager::shouldDisplayHelp() const
{
	return _shouldDisplayHelp;
}

const std::string& Storm::ConfigManager::getSceneName() const
{
	return _sceneFileName;
}

const Storm::SceneData& Storm::ConfigManager::getSceneData() const
{
	return _sceneConfig.getSceneData();
}

const Storm::GraphicData& Storm::ConfigManager::getGraphicData() const
{
	return *_sceneConfig.getSceneData()._graphicData;
}

const Storm::GeneralSimulationData& Storm::ConfigManager::getGeneralSimulationData() const
{
	return *_sceneConfig.getSceneData()._generalSimulationData;
}

const std::vector<Storm::RigidBodySceneData>& Storm::ConfigManager::getRigidBodiesData() const
{
	return _sceneConfig.getSceneData()._rigidBodiesData;
}

const Storm::FluidData& Storm::ConfigManager::getFluidData() const
{
	return *_sceneConfig.getSceneData()._fluidData;
}

const Storm::RecordConfigData& Storm::ConfigManager::getRecordConfigData() const
{
	return *_sceneConfig.getSceneData()._recordConfigData;
}

const std::vector<Storm::BlowerData>& Storm::ConfigManager::getBlowersData() const
{
	return _sceneConfig.getSceneData()._blowersData;
}

const std::vector<Storm::ConstraintData>& Storm::ConfigManager::getConstraintsData() const
{
	return _sceneConfig.getSceneData()._contraintsData;
}

const Storm::RigidBodySceneData& Storm::ConfigManager::getRigidBodyData(unsigned int rbId) const
{
	const std::vector<Storm::RigidBodySceneData> &rbDataArrays = _sceneConfig.getSceneData()._rigidBodiesData;
	if (const auto currentRbDataIter = std::find_if(std::begin(rbDataArrays), std::end(rbDataArrays), [rbId](const Storm::RigidBodySceneData &rb)
	{
		return rb._rigidBodyID == rbId;
	}); currentRbDataIter != std::end(rbDataArrays))
	{
		return *currentRbDataIter;
	}
	else
	{
		Storm::throwException<std::exception>("Cannot find rigid body data with index " + std::to_string(rbId));
	}
}

const std::string& Storm::ConfigManager::getLogFileName() const
{
	return _generalConfig._logFileName;
}

const std::string& Storm::ConfigManager::getLogFolderPath() const
{
	return _generalConfig._logFolderPath;
}

Storm::LogLevel Storm::ConfigManager::getLogLevel() const
{
	return _generalConfig._logLevel;
}

int Storm::ConfigManager::getRemoveLogOlderThanDaysCount() const
{
	return _generalConfig._removeLogsOlderThanDays;
}

bool Storm::ConfigManager::getShouldOverrideOldLog() const
{
	return _generalConfig._overrideLogs;
}

bool Storm::ConfigManager::getShouldLogFpsWatching() const
{
	return _generalConfig._shouldLogFPSWatching;
}

bool Storm::ConfigManager::getShouldLogGraphicDeviceMessage() const
{
	return _generalConfig._shouldLogGraphicDeviceMessage;
}

bool Storm::ConfigManager::getShouldLogPhysics() const
{
	return _generalConfig._shouldLogPhysics;
}
