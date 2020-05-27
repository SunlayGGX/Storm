#include "LoggerManager.h"
#include "LogLevel.h"
#include "LogItem.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "ThreadHelper.h"

#include <iostream>
#include <fstream>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#   include <Windows.h>
#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN


namespace
{
	void logToVisualStudioOutput(const std::string &msg)
	{
		OutputDebugStringA(static_cast<LPCSTR>(msg.c_str()));
	}
}


Storm::LoggerManager::LoggerManager() :
	_level{ Storm::LogLevel::Debug },
	_isRunning{ true }
{
	_buffer.reserve(16);
}

Storm::LoggerManager::~LoggerManager()
{
	this->cleanUp();

	this->writeLogs(_buffer);
	_buffer.clear();
}

void Storm::LoggerManager::initialize_Implementation()
{
	std::vector<std::filesystem::path> logsToBeRemoved;

	std::unique_lock<std::mutex> lock{ _loggerMutex };
	_isRunning = true;

	const Storm::IConfigManager* configMgr = Storm::SingletonHolder::instance().getFacet<Storm::IConfigManager>();
	assert(configMgr != nullptr && "Config Manager should be alive when we initialize the logger!");

	_level = configMgr->getLogLevel();
	int removeLogOlderThanDay = configMgr->getRemoveLogOlderThanDaysCount();

	const std::string &logFileName = configMgr->getLogFileName();
	if (!logFileName.empty())
	{
		const std::filesystem::path logExtension{ ".log" };

		const std::string &logFolderPathStr = configMgr->getLogFolderPath();
		const std::filesystem::path logFolderPath = std::filesystem::path{ logFolderPathStr.empty() ? configMgr->getTemporaryPath() : logFolderPathStr };
		std::filesystem::create_directories(logFolderPath);

		std::filesystem::path logFilePath = logFolderPath / logFileName;
		logFilePath.replace_extension(logExtension);

		if (configMgr->getShouldOverrideOldLog())
		{
			std::filesystem::remove(logFilePath);
		}
		else
		{
			std::ofstream{ _logFilePath, std::ios_base::out | std::ios_base::app } << "\n\n\n---------------------------------------------\n\n\n";
		}

		if (removeLogOlderThanDay > 0)
		{
			const auto threadhold = std::filesystem::file_time_type::clock::now() - std::chrono::hours{ 24 * removeLogOlderThanDay };

			for (const std::filesystem::directory_entry &logsFile : std::filesystem::recursive_directory_iterator{ logFolderPath })
			{
				if (logsFile.path().extension() == logExtension && (logsFile.is_character_file() || logsFile.is_regular_file()))
				{
					auto writeTime = logsFile.last_write_time();
					if (writeTime <= threadhold)
					{
						logsToBeRemoved.emplace_back(logsFile.path());
					}
				}
			}
		}

		_logFilePath = logFilePath.string();
	}

	bool canLeaveTmp = false;
	std::condition_variable syncTmp;

	_loggerThread = std::thread{ [this, sync = &syncTmp, canLeave = &canLeaveTmp]()
	{
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

			this->writeLogs(tmpBuffer);
			tmpBuffer.clear();

			lock.lock();
		}

		this->writeLogs(_buffer);
		_buffer.clear();
	} };

	lock.unlock();

	// Time to log everything we wanted inside this init method.
	LOG_ALWAYS << "Set the log level to '" << Storm::LogItem::parseLogLevel(_level) << "'. It means we wont log message under this level.";
	
	const std::size_t toBeRemovedCount = logsToBeRemoved.size();
	if (toBeRemovedCount > 0)
	{
		LOG_COMMENT << "We would remove " << toBeRemovedCount << " log file (were detected older than " << removeLogOlderThanDay << " days from now).";
		for (const auto &logFilePathToRemove : logsToBeRemoved)
		{
			if (std::filesystem::remove(logFilePathToRemove))
			{
				LOG_DEBUG << logFilePathToRemove << " was removed successfully";
			}
			else
			{
				LOG_DEBUG << "Could not remove " << logFilePathToRemove;
			}
		}
	}

	// Wait for the thread to start before continuing.
	lock.lock();
	syncTmp.wait(lock, [&canLeaveTmp]()
	{
		return canLeaveTmp;
	});
}

void Storm::LoggerManager::cleanUp_Implementation()
{
	LOG_COMMENT << "This is the logger end. No more log will be done afterwards";

	{
		std::lock_guard<std::mutex> lock{ _loggerMutex };
		_isRunning = false;

		_loggerCV.notify_all();
	}

	Storm::join(_loggerThread);
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

void Storm::LoggerManager::writeLogs(const LogArray &logArray) const
{
	const bool debuggerAttached = ::IsDebuggerPresent();

	if (!_logFilePath.empty())
	{
		std::ofstream logFile{ _logFilePath, std::ios_base::out | std::ios_base::app };
		for (const auto &logItem : logArray)
		{
			if (logItem._level >= _level)
			{
				const std::string fullLogMsg{ logItem };
				std::cout << fullLogMsg;
				logFile << fullLogMsg;

				if (debuggerAttached)
				{
					logToVisualStudioOutput(fullLogMsg);
				}
			}
		}
	}
	else
	{
		for (const auto &logItem : logArray)
		{
			if (logItem._level >= _level)
			{
				const std::string fullLogMsg{ logItem };
				std::cout << fullLogMsg;
				if (debuggerAttached)
				{
					logToVisualStudioOutput(fullLogMsg);
				}
			}
		}
	}
}
