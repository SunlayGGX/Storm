#include "ConfigManager.h"
#include "CommandLineParser.h"

#include "SingletonHolder.h"
#include "IOSManager.h"

#include "InternalConfig.h"
#include "GeneralConfig.h"
#include "SceneConfig.h"
#include "SceneRigidBodyConfig.h"

#include "ConfigReadParam.h"

#include "RecordMode.h"
#include "ViscosityMethod.h"

#include "OSHelper.h"

#pragma warning(push)
#pragma warning(disable:4702)
#	include <boost/algorithm/string.hpp>
#pragma warning(pop)


Storm::ConfigManager::ConfigManager() :
	_shouldDisplayHelp{ false },
	_shouldRegenerateParticleCache{ false },
	_withUI{ true },
	_clearLogs{ false }
{

}

Storm::ConfigManager::~ConfigManager() = default;


void Storm::ConfigManager::initialize_Implementation(int argc, const char* argv[])
{
	LOG_DEBUG << "Initializing Configuration";

	// Since internal configurations are for the most part, hard coded auto generated settings.
	// If there is some settings that impact the following initialization, then it is good to have them initialized first before everything else.
	_internalConfigHolder.init();

	Storm::CommandLineParser parser{ argc, argv };
	_clearLogs = parser.clearLogFolder();
	_shouldDisplayHelp = parser.shouldDisplayHelp();
	if (!_shouldDisplayHelp)
	{
		_exePath = argv[0];
		const std::filesystem::path exePath{ _exePath };

		_commandLineForRestart = "--stormPath=";
		_commandLineForRestart += Storm::OSHelper::getRawQuotedCommandline();

		Storm::IOSManager &iosMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IOSManager>();
		_currentPID = iosMgr.obtainCurrentPID();
		_computerName = iosMgr.getComputerName();

		// If is not absolute, make it absolute.
		bool hasMadeAbsolute = !exePath.is_absolute();
		if (hasMadeAbsolute)
		{
			_exePath = (std::filesystem::current_path() / exePath.filename()).string();
		}

		// Get and check the record mode

		std::string recordModeStr = parser.getRecordModeStr();
		boost::algorithm::to_lower(recordModeStr);

		Storm::ConfigReadParam simulationConfigParam;

		simulationConfigParam._simulatorRecordMode =
			recordModeStr == "record" ? Storm::RecordMode::Record :
			recordModeStr == "replay" ? Storm::RecordMode::Replay :
			Storm::RecordMode::None;

		if (!recordModeStr.empty() && simulationConfigParam._simulatorRecordMode != Storm::RecordMode::None)
		{
			switch (simulationConfigParam._simulatorRecordMode)
			{
			case Storm::RecordMode::None:
				LOG_COMMENT << "Simulator in normal mode requested (no record or replay).";
				break;

			case Storm::RecordMode::Record:
				LOG_COMMENT << "Simulator in record mode requested.";
				break;

			case Storm::RecordMode::Replay:
				LOG_COMMENT << "Simulator in replay mode requested.";
				break;

			default:
				Storm::throwException<Storm::Exception>("Unknown record mode command line tag \"" + recordModeStr + '"');
			}
		}

		_userSetThreadPriority = parser.getThreadPriority();

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

		_internalConfigHolder.read(configFolderPath / "Internal" / "InternalConfig.xml", _macroConfig);

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

		_sceneConfigFilePath = _macroConfig(parser.getSceneFilePath());
		if (_sceneConfigFilePath.empty())
		{
			LOG_COMMENT << "No scene file passed from command line. Will ask user the config file with an explorer file dialog.";

			const std::map<std::wstring, std::wstring> fileFilters{
				{ L"Xml file (*.xml)", L"*.xml" }
			};

			_sceneConfigFilePath = Storm::toStdString(iosMgr.openFileExplorerDialog(_defaultSceneConfigFolderPath, fileFilters));

			_commandLineForRestart += parser.getSceneFilePathTag();
			_commandLineForRestart += "=\"";
			_commandLineForRestart += _sceneConfigFilePath;
			_commandLineForRestart += '"';
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

			_macroConfig.registerMacro("SceneName", _sceneFileName);
			_macroConfig.registerMacro("SceneStateFolder", (std::filesystem::path{ *_macroConfig.queryMacroValue("StormStates") } / _sceneFileName).string());
			_macroConfig.resolveInternalMacro();

			const std::string generalConfigFilePathStrFromCmdLine = _macroConfig(parser.getGeneralConfigFilePath());
			if (generalConfigFilePathStrFromCmdLine.empty() || !_generalConfigHolder.read(generalConfigFilePathStrFromCmdLine))
			{
				_generalConfigHolder.read(defaultGeneralConfigFilePath.string());
			}

			// Now that we have read the general config and know what to do with vectored exception, set it here.
			Storm::setLogVectoredExceptionsDisplayMode(this->getGeneralDebugConfig()._displayVectoredExceptions);

			_generalConfigHolder.applyMacros(_macroConfig);

			_sceneConfigHolder.read(_sceneConfigFilePath, _macroConfig, _generalConfigHolder, simulationConfigParam);
		}
		else
		{
			LOG_FATAL << errorMsg;
			Storm::throwException<Storm::Exception>(errorMsg);
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
			Storm::throwException<Storm::Exception>(errorMsg);
		}

		_shouldRegenerateParticleCache = parser.getShouldRegenerateParticleCache();

		_stateFileToLoad = _macroConfig(parser.getStateFilePath());

		const bool noLoadPhysicsTime = parser.noPhysicsTimeLoad();
		const bool noForcesLoadSpecified = parser.noForceLoad();
		const bool noVelocitiesLoadSpecified = parser.noVelocityLoad();
		
		if (_stateFileToLoad.empty())
		{
			if (noLoadPhysicsTime || noForcesLoadSpecified || noVelocitiesLoadSpecified)
			{
				Storm::throwException<Storm::Exception>(
					"Some command line flags referring to state file loading were used while we haven't specified a state file to load from.\n"
					"It is forbidden!"
				);
			}
		}
		else
		{
			LOG_DEBUG << "State loading requested! File to load is " << _stateFileToLoad;
		}

		Storm::SceneConfig &sceneConfig = _sceneConfigHolder.getConfig();

		_loadPhysicsTime = !noLoadPhysicsTime;
		_loadForces = !noForcesLoadSpecified;
		_loadVelocities = !noVelocitiesLoadSpecified;

		Storm::SceneRecordConfig &sceneRecordConfig = sceneConfig._recordConfig;
		sceneRecordConfig._recordMode = simulationConfigParam._simulatorRecordMode;

		std::string recordFilePath;
		switch (sceneRecordConfig._recordMode)
		{
		case Storm::RecordMode::Record:
		{
			recordFilePath = parser.getRecordFilePath();
			if (!recordFilePath.empty())
			{
				sceneRecordConfig._recordFilePath = std::move(recordFilePath);
			}
			_macroConfig(sceneRecordConfig._recordFilePath);
			if (sceneRecordConfig._recordFilePath.empty())
			{
				Storm::throwException<Storm::Exception>("Record file path should be set when in record mode!");
			}
			else if (sceneRecordConfig._recordFps == -1.f) // Check manually set invalid values was done before. Here we check the unset.
			{
				Storm::throwException<Storm::Exception>("Record fps wasn't set while we should be recording. We should always set one!");
			}

			const std::filesystem::path recordFilePathFs{ sceneRecordConfig._recordFilePath };
			std::filesystem::remove_all(recordFilePathFs);
			std::filesystem::create_directories(recordFilePathFs.parent_path());
			break;
		}

		case Storm::RecordMode::Replay:
		{
			recordFilePath = parser.getRecordFilePath();
			if (!recordFilePath.empty())
			{
				sceneRecordConfig._recordFilePath = std::move(recordFilePath);
			}
			_macroConfig(sceneRecordConfig._recordFilePath);
			if (!std::filesystem::is_regular_file(sceneRecordConfig._recordFilePath))
			{
				Storm::throwException<Storm::Exception>(sceneRecordConfig._recordFilePath + " doesn't exist or isn't a regular record file!");
			}

			Storm::SceneSimulationConfig &sceneSimulationConfig = sceneConfig._simulationConfig;
			if (sceneRecordConfig._replayRealTime && sceneSimulationConfig._simulationNoWait)
			{
				LOG_WARNING <<
					"replayRealTime and simulationNoWait are both enabled.\n"
					"These are 2 opposite flags, except the simulationNoWait is general to all modes while replayRealTime is only for replay mode.\n"
					"Therefore, since we are in replay mode. replayRealTime take precedence.";

				sceneSimulationConfig._simulationNoWait = false;
			}

			// It doesn't make sense to start from a state file when we replay, since a replay is a bunch of state file that forces the simulation to behave exactly as what was recorded.
			if (!_stateFileToLoad.empty())
			{
				Storm::throwException<Storm::Exception>("We cannot load a state file in replay mode.");
			}

			break;
		}

		case Storm::RecordMode::None:
		default:
			recordFilePath = parser.getRecordFilePath();
			if (!recordFilePath.empty())
			{
				Storm::throwException<Storm::Exception>("A record file from command line has been set (" + recordFilePath + "). But we're not recording or replaying, it is forbidden!");
			}
			// No need to check the one coming from the scene config file because it is just a placeholder in case the command line one isn't set.
			break;
		}

		const bool noUI = parser.getNoUI();
		_withUI = !noUI;

		if (noUI)
		{
			if (simulationConfigParam._simulatorRecordMode != Storm::RecordMode::Record)
			{
				if (simulationConfigParam._simulatorRecordMode == Storm::RecordMode::Replay)
				{
					Storm::throwException<Storm::Exception>("We must always have a UI when replaying a simulation.");
				}

				const Storm::GeneralDebugConfig &debugConfig = this->getGeneralDebugConfig();
				const bool intendProfilingWithoutUI = debugConfig._profileSimulationSpeed && sceneConfig._simulationConfig._endSimulationPhysicsTimeInSeconds > 0.f;
				if (intendProfilingWithoutUI)
				{
					LOG_DEBUG_WARNING << 
						"No UI mode set with some profiling flags, therefore we expect the reason was to profile.\n"
						"However, beware that some features are disabled in this mode so we would get partial profiling data (only simulator related features would run).";
				}
				else
				{
					Storm::throwException<Storm::Exception>("When starting without a UI means that it is focused on recording or profiling! We must either set recording mode or profile the simulator.");
				}
			}

			bool &startPaused = sceneConfig._simulationConfig._startPaused;
			if (startPaused)
			{
				startPaused = false;
				LOG_WARNING << "Without UI, since there is no input, we cannot start paused (the simulation will start automatically once ready).";
			}
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

bool Storm::ConfigManager::withUI() const
{
	return _withUI;
}

Storm::ThreadPriority Storm::ConfigManager::getUserSetThreadPriority() const
{
	return _userSetThreadPriority;
}

const std::string& Storm::ConfigManager::getStateFilePath() const
{
	return _stateFileToLoad;
}

void Storm::ConfigManager::stateShouldLoad(bool &outLoadPhysicsTime, bool &outLoadForces, bool &outLoadVelocities) const
{
	outLoadPhysicsTime = _loadPhysicsTime;
	outLoadForces = _loadForces;
	outLoadVelocities = _loadVelocities;
}

bool Storm::ConfigManager::clearAllLogs() const
{
	return _clearLogs;
}

bool Storm::ConfigManager::shouldDisplayHelp() const
{
	return _shouldDisplayHelp;
}

const Storm::GeneratedGitConfig& Storm::ConfigManager::getInternalGeneratedGitConfig() const
{
	return *_internalConfigHolder.getInternalConfig()._generatedGitConfig;
}

const std::vector<Storm::InternalReferenceConfig>& Storm::ConfigManager::getInternalReferencesConfig() const
{
	return _internalConfigHolder.getInternalConfig()._internalReferencesConfig;
}

const Storm::GeneralGraphicConfig& Storm::ConfigManager::getGeneralGraphicConfig() const
{
	return _generalConfigHolder.getConfig()._generalGraphicConfig;
}

const Storm::GeneralSimulationConfig& Storm::ConfigManager::getGeneralSimulationConfig() const
{
	return _generalConfigHolder.getConfig()._generalSimulationConfig;
}

const Storm::GeneralNetworkConfig& Storm::ConfigManager::getGeneralNetworkConfig() const
{
	return _generalConfigHolder.getConfig()._generalNetworkConfig;
}

const Storm::GeneralWebConfig& Storm::ConfigManager::getGeneralWebConfig() const
{
	return _generalConfigHolder.getConfig()._generalWebConfig;
}

const Storm::GeneralDebugConfig& Storm::ConfigManager::getGeneralDebugConfig() const
{
	return _generalConfigHolder.getConfig()._generalDebugConfig;
}

const Storm::GeneralApplicationConfig& Storm::ConfigManager::getGeneralApplicationConfig() const
{
	return _generalConfigHolder.getConfig()._generalApplicationConfig;
}

const Storm::GeneralSafetyConfig& Storm::ConfigManager::getGeneralSafetyConfig() const
{
	return _generalConfigHolder.getConfig()._generalSafetyConfig;
}

const Storm::GeneralArchiveConfig& Storm::ConfigManager::getGeneralArchiveConfig() const
{
	return _generalConfigHolder.getConfig()._generalArchiveConfig;
}

const Storm::SceneGraphicConfig& Storm::ConfigManager::getSceneGraphicConfig() const
{
	return _sceneConfigHolder.getConfig()._graphicConfig;
}

const Storm::SceneSimulationConfig& Storm::ConfigManager::getSceneSimulationConfig() const
{
	return _sceneConfigHolder.getConfig()._simulationConfig;
}

const Storm::ScenePhysicsConfig& Storm::ConfigManager::getScenePhysicsConfig() const
{
	return _sceneConfigHolder.getConfig()._physicsConfig;
}

const std::vector<Storm::SceneRigidBodyConfig>& Storm::ConfigManager::getSceneRigidBodiesConfig() const
{
	return _sceneConfigHolder.getConfig()._rigidBodiesConfig;
}

const Storm::SceneFluidConfig& Storm::ConfigManager::getSceneFluidConfig() const
{
	return _sceneConfigHolder.getConfig()._fluidConfig;
}

const Storm::SceneRecordConfig& Storm::ConfigManager::getSceneRecordConfig() const
{
	return _sceneConfigHolder.getConfig()._recordConfig;
}

const std::vector<Storm::SceneBlowerConfig>& Storm::ConfigManager::getSceneBlowersConfig() const
{
	return _sceneConfigHolder.getConfig()._blowersConfig;
}

const std::vector<Storm::SceneConstraintConfig>& Storm::ConfigManager::getSceneConstraintsConfig() const
{
	return _sceneConfigHolder.getConfig()._contraintsConfig;
}

const Storm::SceneRigidBodyConfig& Storm::ConfigManager::getSceneRigidBodyConfig(unsigned int rbId) const
{
	const std::vector<Storm::SceneRigidBodyConfig> &rbConfigArrays = _sceneConfigHolder.getConfig()._rigidBodiesConfig;
	if (const auto currentRbConfigIter = std::find_if(std::begin(rbConfigArrays), std::end(rbConfigArrays), [rbId](const Storm::SceneRigidBodyConfig &rb)
	{
		return rb._rigidBodyID == rbId;
	}); currentRbConfigIter != std::end(rbConfigArrays))
	{
		return *currentRbConfigIter;
	}
	else
	{
		Storm::throwException<Storm::Exception>("Cannot find rigid body data with index " + std::to_string(rbId));
	}
}

const Storm::SceneScriptConfig& Storm::ConfigManager::getSceneScriptConfig() const
{
	return _sceneConfigHolder.getConfig()._scriptConfig;
}

const Storm::SceneCageConfig* Storm::ConfigManager::getSceneOptionalCageConfig() const
{
	return _sceneConfigHolder.getConfig()._optionalCageConfig.get();
}

bool Storm::ConfigManager::isInReplayMode() const noexcept
{
	return _sceneConfigHolder.getConfig()._recordConfig._recordMode == Storm::RecordMode::Replay;
}

bool Storm::ConfigManager::isInRecordMode() const noexcept
{
	return _sceneConfigHolder.getConfig()._recordConfig._recordMode == Storm::RecordMode::Record;
}

bool Storm::ConfigManager::userCanModifyTimestep() const noexcept
{
	return !(this->isInReplayMode() || this->getSceneSimulationConfig()._computeCFL);
}

const std::string& Storm::ConfigManager::getSceneName() const
{
	return _sceneFileName;
}

const std::string& Storm::ConfigManager::getSimulationTypeName() const
{
	return this->getSceneSimulationConfig()._simulationModeStr;
}

std::string Storm::ConfigManager::getViscosityMethods() const
{
	struct ViscosityMethodParserPolicy
	{
		static std::string parsePolicyAgnostic(const Storm::ViscosityMethod viscoMethod)
		{
#define STORM_PARSE(viscoMethValue) case Storm::ViscosityMethod::viscoMethValue: return STORM_STRINGIFY(viscoMethValue)
			switch (viscoMethod)
			{
				STORM_PARSE(XSPH);
				STORM_PARSE(Standard);

			default:
				Storm::throwException<Storm::Exception>("Viscosity method value is unknown : '" + Storm::toStdString(viscoMethod) + "'");
			}
#undef STORM_PARSE
		}
	};

	std::string result;
	
	const Storm::SceneSimulationConfig &simulConfig = this->getSceneSimulationConfig();

	const std::string fluidViscoStr = Storm::toStdString<ViscosityMethodParserPolicy>(simulConfig._fluidViscoMethod);
	const std::string rbViscoStr = Storm::toStdString<ViscosityMethodParserPolicy>(simulConfig._rbViscoMethod);

	result.reserve(24 + fluidViscoStr.size() + rbViscoStr.size());

	result += "FluidVisco:";
	result += fluidViscoStr;
	result += " - RbVisco:";
	result += rbViscoStr;

	return result;
}

unsigned int Storm::ConfigManager::getCurrentPID() const
{
	return _currentPID;
}


const std::string& Storm::ConfigManager::getComputerName() const
{
	return _computerName;
}

const std::string& Storm::ConfigManager::getSceneConfigFilePath() const
{
	return _sceneConfigFilePath;
}

const std::string& Storm::ConfigManager::getScriptFilePath() const
{
	return this->getSceneScriptConfig()._scriptFilePipe._filePath;
}

const std::string& Storm::ConfigManager::getRestartCommandline() const
{
	return _commandLineForRestart;
}

bool Storm::ConfigManager::hasWall() const
{
	const std::vector<Storm::SceneRigidBodyConfig> &rbConfigs = this->getSceneRigidBodiesConfig();
	return std::ranges::any_of(rbConfigs, [](const Storm::SceneRigidBodyConfig &rbConfig) { return rbConfig._isWall; });
}

bool Storm::ConfigManager::shouldArchive() const
{
	return this->isInRecordMode() && this->getGeneralArchiveConfig()._enabled;
}

void Storm::ConfigManager::getUnsafeMacroizedConvertedValue(std::string &inOutValue) const
{
	_macroConfig(inOutValue);
}


bool Storm::ConfigManager::getMaybeMacroizedConvertedValue(std::string &inOutValue) const
{
	std::size_t firstTagPosHint;
	if (_macroConfig.hasKnownMacro(inOutValue, firstTagPosHint))
	{
		_macroConfig(inOutValue, firstTagPosHint);
		return true;
	}

	return false;
}

void Storm::ConfigManager::getMacroizedConvertedValue(std::string &inOutValue) const
{
	if (!this->getMaybeMacroizedConvertedValue(inOutValue))
	{
		Storm::throwException<Storm::Exception>("'" + inOutValue + "' has no known macros embedded into it!");
	}
}

std::string Storm::ConfigManager::makeMacroKey(const std::string_view value) const
{
	return _macroConfig.makeMacroKey(value);
}
