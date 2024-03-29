#include "LoggerManager.h"
#include "LogLevel.h"
#include "LogItem.h"
#include "LogHelper.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"
#include "IThreadManager.h"
#include "ISerializerManager.h"

#include "GeneralDebugConfig.h"

#include "ThreadHelper.h"
#include "ThreadEnumeration.h"
#include "ThreadFlaggerObject.h"
#include "ThreadingSafety.h"

#include "LeanWindowsInclude.h"

#include "Config/MacroTags.cs"

#include <iostream>
#include <fstream>


namespace
{
	void logToVisualStudioOutput(const std::string &msg)
	{
		::OutputDebugStringA(static_cast<LPCSTR>(msg.c_str()));
	}

	std::filesystem::path computeXmlLogFilePath(const Storm::IConfigManager* configMgr, const Storm::GeneralDebugConfig &generalDebugConfig, const std::string &logFileName, const std::filesystem::path &xmlLogExtension, std::filesystem::path &outFolderPath)
	{
		const std::string &logFolderPathStr = generalDebugConfig._logFolderPath;
		outFolderPath = std::filesystem::path{ logFolderPathStr.empty() ? configMgr->getTemporaryPath() : logFolderPathStr };
		std::filesystem::create_directories(outFolderPath);

		std::filesystem::path logFilePath = outFolderPath / logFileName;
		return std::filesystem::path{ logFilePath }.replace_extension(xmlLogExtension);
	}

	std::filesystem::path computeXmlMacroizedLogFilePath(const Storm::IConfigManager &configMgr, const Storm::GeneralDebugConfig &generalDebugConfig, const std::string &logFileName, const std::filesystem::path &xmlLogExtension)
	{
		const std::string &logFolderPathStr = generalDebugConfig._srcLogFolderPath;
		const std::filesystem::path outFolderPath{ logFolderPathStr.empty() ? configMgr.makeMacroKey(Storm::MacroTags::k_builtInMacroKey_StormTmp) : logFolderPathStr };
		
		std::filesystem::path logFilePath = outFolderPath / logFileName;
		return std::filesystem::path{ logFilePath }.replace_extension(xmlLogExtension);
	}
}


Storm::LoggerManager::LoggerManager() :
	_level{ Storm::LogLevel::Debug },
	_isRunning{ true },
	_currentPID{ 0 }
{
	_buffer.reserve(16);
}

Storm::LoggerManager::~LoggerManager()
{
	this->cleanUp();

	// Pseudo initialization in case we logged something between the time the app was started, but before we initialized the LoggerManager but we stopped code execution in between due to something (like an exception for example).
	// Since this needs the config manager to be minimally initialized at least (and maybe the throw happened while initializing it, therefore it was maybe mid init), we're taking some caution.
	if (_xmlLogFilePath.empty())
	{
		if (Storm::SingletonHolder::isAlive())
		{
			const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

			const Storm::IConfigManager* configMgr = singletonHolder.getFacet<Storm::IConfigManager>();
			if (configMgr != nullptr)
			{
				if (_currentPID == 0)
				{
					_currentPID = configMgr->getCurrentPID();
				}

				const Storm::GeneralDebugConfig &generalDebugConfig = configMgr->getGeneralDebugConfig();

				const std::string &logFileName = generalDebugConfig._logFileName;
				if (!logFileName.empty())
				{
					const std::filesystem::path xmlLogExtension{ ".xml" };

					std::filesystem::path logFolderPath;
					const std::filesystem::path xmlLogFilePath = computeXmlLogFilePath(configMgr, generalDebugConfig, logFileName, xmlLogExtension, logFolderPath);

					_xmlLogFilePath = xmlLogFilePath.string();
				}
			}
		}
	}

	STORM_DECLARE_THIS_THREAD_IS << Storm::ThreadFlagEnum::LoggingThread;

	this->writeLogs(_buffer, _currentPID);
	_buffer.clear();
}

void Storm::LoggerManager::initialize_Implementation()
{
	std::vector<std::filesystem::path> logsToBeRemoved;

	std::unique_lock<std::mutex> lock{ _loggerMutex };

#define STORM_PRODUCE_INIT_LOGGING_UNDER_LOCK(LogDeclaration)	\
lock.unlock();													\
LogDeclaration													\
lock.lock()														\

	_isRunning = true;

	const Storm::IConfigManager* configMgr = Storm::SingletonHolder::instance().getFacet<Storm::IConfigManager>();
	assert(configMgr != nullptr && "Config Manager should be alive when we initialize the logger!");

	const Storm::GeneralDebugConfig &generalDebugConfig = configMgr->getGeneralDebugConfig();

	_level = generalDebugConfig._logLevel;
	int removeLogOlderThanDay = generalDebugConfig._removeLogsOlderThanDays;

	const std::string &logFileName = generalDebugConfig._logFileName;
	if (!logFileName.empty())
	{
		const std::filesystem::path xmlLogExtension{ ".xml" };
		
		std::filesystem::path logFolderPath;
		const std::filesystem::path xmlLogFilePath = computeXmlLogFilePath(configMgr, generalDebugConfig, logFileName, xmlLogExtension, logFolderPath);

		_xmlLogFilePath = xmlLogFilePath.string();

		if (!configMgr->clearAllLogs())
		{
			if (generalDebugConfig._overrideLogs)
			{
				std::filesystem::remove(xmlLogFilePath);
			}
			else
			{
				std::ofstream{ _xmlLogFilePath, std::ios_base::out | std::ios_base::app } << "<separator/>";
			}

			if (removeLogOlderThanDay > 0)
			{
				const auto threadhold = std::filesystem::file_time_type::clock::now() - std::chrono::hours{ 24 * removeLogOlderThanDay };

				for (const std::filesystem::directory_entry &logsFile : std::filesystem::recursive_directory_iterator{ logFolderPath })
				{
					const std::filesystem::path currentExtension = logsFile.path().extension();
					if (currentExtension == xmlLogExtension && (logsFile.is_character_file() || logsFile.is_regular_file()))
					{
						auto writeTime = logsFile.last_write_time();
						if (writeTime <= threadhold)
						{
							logsToBeRemoved.emplace_back(logsFile.path());
						}
					}
				}
			}
		}
		else
		{
			STORM_PRODUCE_INIT_LOGGING_UNDER_LOCK(
				LOG_DEBUG << "Clear all logs flag was triggered, therefore we'll empty the log folder before proceeding.";
			);

			try
			{
				std::filesystem::remove_all(logFolderPath);
				std::filesystem::create_directories(logFolderPath);
			}
			catch (const std::exception &ex)
			{
				STORM_PRODUCE_INIT_LOGGING_UNDER_LOCK(
					LOG_ERROR << "Cannot clean '" << logFolderPath << "' as requested by command line flag. Reason was " << ex.what();
				);
			}
		}
	}

	try
	{
		_tmpLogFolderPath = configMgr->getTemporaryPath();
		_tmpLogFolderPath /= "TmpLogs";
		std::filesystem::create_directories(_tmpLogFolderPath);
	}
	catch (const std::exception &ex)
	{
		STORM_PRODUCE_INIT_LOGGING_UNDER_LOCK(
			LOG_ERROR << "Cannot create tmp log folder path. Reason was " << ex.what();
		);
	}

	_currentPID = configMgr->getCurrentPID();

	bool canLeaveTmp = false;
	std::condition_variable syncTmp;

	_loggerThread = std::thread{ [this, sync = &syncTmp, canLeave = &canLeaveTmp]()
	{
		STORM_REGISTER_THREAD(LoggerThread);
		STORM_DECLARE_THIS_THREAD_IS << Storm::ThreadFlagEnum::LoggingThread;

		Storm::LoggerManager::LogArray tmpBuffer;

		std::unique_lock<std::mutex> lock{ _loggerMutex };
		*canLeave = true;
		sync->notify_all();

		while (!_loggerCV.wait_for(lock, std::chrono::milliseconds{ 500 }, [this]() { return !_isRunning; }))
		{
			if (_buffer.empty())
			{
				continue;
			}

			std::swap(_buffer, tmpBuffer);

			lock.unlock();

			this->writeLogs(tmpBuffer, _currentPID);
			tmpBuffer.clear();

			lock.lock();
		}

		this->writeLogs(_buffer, _currentPID);
		_buffer.clear();
	} };

	lock.unlock();

	// Time to log everything we wanted inside this init method.
	LOG_ALWAYS << "Set the log level to '" << Storm::parseLogLevel(_level) << "'. It means we wont log message under this level.";
	
	const std::size_t toBeRemovedCount = logsToBeRemoved.size();
	if (toBeRemovedCount > 0)
	{
		LOG_COMMENT << "We would remove " << toBeRemovedCount << " log file (were detected older than " << removeLogOlderThanDay << " days from now).";

		const std::size_t expectedLengthNoRealloc = (4 + 128) * toBeRemovedCount;

		std::string removed;
		std::string notRemoved;
		removed.reserve(expectedLengthNoRealloc);
		notRemoved.reserve(expectedLengthNoRealloc / 4); // Less because it is exceptional, so we estimate that it should be far less than the real realloc.

		for (const auto &logFilePathToRemove : logsToBeRemoved)
		{
			if (std::filesystem::remove(logFilePathToRemove))
			{
				removed += "\n\t- ";
				removed += Storm::toStdString(logFilePathToRemove);
			}
			else
			{
				notRemoved += "\n\t- ";
				notRemoved += Storm::toStdString(logFilePathToRemove);
			}
		}

		if (!removed.empty())
		{
			LOG_DEBUG << "We successfully removed those logs :" << removed;
		}
		if (!notRemoved.empty())
		{
			LOG_ERROR << "We failed to remove those logs :" << notRemoved;
		}
	}
	else
	{
		LOG_DEBUG << "No logs to remove.";
	}

	// Wait for the thread to start before continuing.
	lock.lock();
	syncTmp.wait(lock, [&canLeaveTmp]()
	{
		return canLeaveTmp;
	});

#undef STORM_PRODUCE_INIT_LOGGING_UNDER_LOCK
}

void Storm::LoggerManager::cleanUp_Implementation()
{
	LOG_COMMENT << "This is the logger end. No more log will be done afterwards";

	{
		std::lock_guard<std::mutex> lock{ _loggerMutex };
		_isRunning = false;
	}

	_loggerCV.notify_all();

	Storm::join(_loggerThread);

	STORM_DECLARE_THIS_THREAD_IS << Storm::ThreadFlagEnum::LoggingThread;

	// Archive the logs if needed.
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

	if (configMgr.shouldArchive())
	{
		const Storm::ISerializerManager &serializerMgr = singletonHolder.getSingleton<Storm::ISerializerManager>();
		std::filesystem::path archiveFolderPath{ serializerMgr.getArchivePath() };
		if (!archiveFolderPath.empty())
		{
			const std::filesystem::path xmlLogFilePath{ _xmlLogFilePath };
			const std::filesystem::path xmlLogFileName = xmlLogFilePath.filename();

			try
			{
				std::filesystem::copy_file(xmlLogFilePath, archiveFolderPath / xmlLogFileName);
			}
			catch (...)
			{

			}

			const Storm::GeneralDebugConfig &generalDebugConfig = configMgr.getGeneralDebugConfig();
			const std::filesystem::path xmlLogExtension{ ".xml" };

			std::ofstream batFile{ archiveFolderPath / "LoggerReader.bat" };
			batFile <<
				"echo off\n"
				"cd %~dp0\n\n"
				"cd ../../../../bin/Release\n\n\n"

				"call \"Storm-LogViewer.exe\" NoKeepUp LogFilePath=" << computeXmlMacroizedLogFilePath(configMgr, generalDebugConfig, Storm::toStdString(xmlLogFileName), xmlLogFileName.extension());
		}
	}
}

void Storm::LoggerManager::log(const std::string_view &moduleName, Storm::LogLevel level, const std::string_view &function, const int line, std::string &&msg)
{
	std::lock_guard<std::mutex> lock{ _loggerMutex };
	if (_isRunning)
	{
		_buffer.emplace_back(moduleName, level, function, line, std::move(msg));
	}
}

Storm::LogLevel Storm::LoggerManager::getLogLevel() const
{
	return _level;
}

void Storm::LoggerManager::writeLogs(LogArray &logArray, const unsigned int currentPID) const
{
	assert(Storm::isLoggerThread() && "This method can only be called from logging thread.");

	const bool debuggerAttached = ::IsDebuggerPresent();

	if (!_xmlLogFilePath.empty())
	{
		std::ofstream xmlLogFile{ _xmlLogFilePath, std::ios_base::out | std::ios_base::app };
		for (auto &logItem : logArray)
		{
			if (logItem._level >= _level)
			{
				logItem.prepare(true, currentPID);

				const std::string &fullLogMsg = logItem.rawMessage();
				std::cout << fullLogMsg;

				if (debuggerAttached)
				{
					logToVisualStudioOutput(fullLogMsg);
				}

				xmlLogFile << logItem.toXml();
			}
		}
	}
	else
	{
		for (auto &logItem : logArray)
		{
			if (logItem._level >= _level)
			{
				logItem.prepare(true, currentPID);

				const std::string &fullLogMsg = logItem.rawMessage();
				std::cout << fullLogMsg;

				if (debuggerAttached)
				{
					logToVisualStudioOutput(fullLogMsg);
				}
			}
		}
	}
}

void Storm::LoggerManager::logToTempFile(const std::string &fileName, const std::string &msg) const
{
	std::ofstream{ _tmpLogFolderPath / fileName} << msg;
}
