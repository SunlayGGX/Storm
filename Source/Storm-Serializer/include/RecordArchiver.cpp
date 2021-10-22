#include "RecordArchiver.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "GeneralArchiveConfig.h"
#include "SceneRecordConfig.h"

#include "Config/MacroTags.cs"

#include <fstream>


namespace
{
	struct FilesystemRequest
	{
	public:
		std::filesystem::path _from;
		std::filesystem::path _toFilename;
		bool _move;
	};

	FilesystemRequest computeCopyRequest(const std::string &versionAppendStr, const std::string &filePath, const bool move)
	{
		FilesystemRequest result{
			._from = filePath,
			._move = move
		};

		result._toFilename = result._from.filename();

		const std::filesystem::path ext = result._toFilename.extension();
		result._toFilename.replace_extension();
		result._toFilename += versionAppendStr;
		result._toFilename.replace_extension(ext);

		return result;
	}

	void generateLauncherRunner(const FilesystemRequest &sceneConfigRequest, const FilesystemRequest &recordRequest, const std::filesystem::path &endFolder, const std::filesystem::path &endFolderUniformMacroized)
	{
		std::filesystem::path runnerFileName = sceneConfigRequest._toFilename;
		runnerFileName.replace_extension(".bat");

		std::ofstream batFile{ endFolder / runnerFileName };
		batFile <<
			"echo off\n"
			"cd %~dp0\n\n"
			"cd ../../../../bin/Release\n\n\n"

			//"call \"../Base/LauncherSetup.bat\" Release\n\n\n"

			"call \"Storm.exe\" --threadPriority=High --mode=Replay ";

		batFile << 
			"--scene=" << endFolderUniformMacroized / sceneConfigRequest._toFilename << " "
			"--recordFile=" << endFolderUniformMacroized / recordRequest._toFilename << " ";
	}

	std::filesystem::path retrieveMacroizedRecordArchiveFolder(const Storm::IConfigManager &configMgr)
	{
		return std::filesystem::path{ configMgr.makeMacroKey(Storm::MacroTags::k_builtInMacroKey_StormArchive) } / "Records";
	}
}


Storm::RecordArchiver::RecordArchiver() :
	_version{ 0 }
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

	if (!configMgr.isInRecordMode())
	{
		Storm::throwException<Storm::Exception>("Record archiver cannot be created outside record mode!");
	}

	LOG_DEBUG << "Record archiver created.";

	_archivePath = Storm::toStdString(retrieveMacroizedRecordArchiveFolder(configMgr));
	configMgr.getMacroizedConvertedValue(_archivePath);

	const std::filesystem::path archivePathFilesystem{ _archivePath };
	std::filesystem::create_directories(archivePathFilesystem);

	const std::string &sceneName = configMgr.getSceneName();

	constexpr std::string_view versionStrIdentifier = "_v";

	const bool looseVersionning = !generalArchiveConfig._strictVersionning;

	// Research a new record folder to write to.
	for (const auto dirIter : std::filesystem::directory_iterator{ archivePathFilesystem })
	{
		if (dirIter.is_directory())
		{
			const std::string existingArchiveDirName = Storm::toStdString(dirIter.path().filename());
			if (const std::size_t versionPos = existingArchiveDirName.find_last_of(versionStrIdentifier); versionPos != std::string::npos)
			{
				if (versionPos > 0)
				{
					const std::string_view nameNoComputerName{
						existingArchiveDirName.c_str(),
						existingArchiveDirName.c_str() + existingArchiveDirName.find_first_of('_')
					};

					if (nameNoComputerName == sceneName)
					{
						const char*const end = existingArchiveDirName.c_str() + existingArchiveDirName.size();
						const std::string_view existingArchiveDirVersionStr{ existingArchiveDirName.c_str() + versionPos + 1, end };

						decltype(_version) parsedExistingVersionValue;
						char* endParsing;
						parsedExistingVersionValue = static_cast<decltype(parsedExistingVersionValue)>(std::strtoul(existingArchiveDirVersionStr.data(), &endParsing, 10));
						if (looseVersionning || endParsing == end)
						{
							_version = std::max(parsedExistingVersionValue, _version);
						}
					}
				}
			}
		}
	}

	++_version;
	std::string versionStr = std::to_string(_version);

	std::string newArchiveName;
	newArchiveName.reserve(versionStr.size() + sceneName.size() + versionStrIdentifier.size());

	newArchiveName += sceneName;
	newArchiveName += '_';
	newArchiveName += configMgr.getComputerName();
	newArchiveName += versionStrIdentifier;
	newArchiveName += versionStr;
	
	const std::filesystem::path newArchiveNameFilesystem = std::filesystem::path{ _archivePath } / newArchiveName;

	_archivePath = Storm::toStdString(newArchiveNameFilesystem);
	std::filesystem::create_directories(newArchiveNameFilesystem);

	if (!std::filesystem::is_directory(newArchiveNameFilesystem))
	{
		Storm::throwException<Storm::Exception>("Cannot create archive folder " + _archivePath + ". No archive could be made.");
	}
}

void Storm::RecordArchiver::execute()
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

	const Storm::GeneralArchiveConfig &generalArchiveConfig = configMgr.getGeneralArchiveConfig();
	if (generalArchiveConfig._enabled)
	{
		LOG_DEBUG << "Archiving start.";

		const std::string versionAppendStr = " v" + std::to_string(_version);

		// If the debugger is attached, we're debugging right now and the command lines are given through visual studio.
		// We don't want to change the recordings path inside Visual Studio command line to troubleshoot what we just recorded right now.
		// So we prefer copying the files instead of moving them.
		const bool canMoveFiles = !Storm::isDebuggerAttached();

		FilesystemRequest requests[] =
		{
			computeCopyRequest(versionAppendStr,  configMgr.getSceneConfigFilePath(), false),
			computeCopyRequest(versionAppendStr, configMgr.getSceneRecordConfig()._recordFilePath, canMoveFiles),
		};

		const std::filesystem::path endFolder{ _archivePath };
		for (const auto &request : requests)
		{
			const std::filesystem::path finalDst = endFolder / request._toFilename;
			if (request._move)
			{
				std::filesystem::rename(request._from, finalDst);
				LOG_DEBUG << "Moved " << request._from << " to " << finalDst;
			}
			else
			{
				std::filesystem::copy_file(request._from, finalDst, std::filesystem::copy_options::overwrite_existing);
				LOG_DEBUG << "Copied " << request._from << " to " << finalDst;
			}
		}

		LOG_DEBUG << "Generating runner launcher.";

		const std::filesystem::path endFolderNormalizedWithMacros = std::filesystem::path{ retrieveMacroizedRecordArchiveFolder(configMgr) } / endFolder.filename();
		generateLauncherRunner(requests[0], requests[1], endFolder, endFolderNormalizedWithMacros);

		LOG_COMMENT << "Archiving finished successfully.";
	}
	else
	{
		LOG_DEBUG_WARNING << "No archiving since record archiving was disabled in the general config.";
	}
}
