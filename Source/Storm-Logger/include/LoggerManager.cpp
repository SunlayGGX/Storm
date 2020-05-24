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
	std::unique_lock<std::mutex> lock{ _loggerMutex };

	const Storm::IConfigManager* configMgr = Storm::SingletonHolder::instance().getFacet<Storm::IConfigManager>();
	assert(configMgr != nullptr && "Config Manager should be alive when we initialize the logger!");

	const std::string &logFileName = configMgr->getLogFileName();
	if (!logFileName.empty())
	{
		const std::filesystem::path logFilePath = std::filesystem::path{ configMgr->getTemporaryPath() } / logFileName;
		std::filesystem::remove(logFilePath);

		_logFilePath = logFilePath.string();
	}

	bool canLeaveTmp = false;
	std::condition_variable syncTmp;

	_isRunning = true;
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

	syncTmp.wait(lock, [&canLeaveTmp]()
	{
		return canLeaveTmp;
	});
}

void Storm::LoggerManager::cleanUp_Implementation()
{
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
			const std::string fullLogMsg{ logItem };
			std::cout << fullLogMsg;
			logFile << fullLogMsg;

			if (debuggerAttached)
			{
				logToVisualStudioOutput(fullLogMsg);
			}
		}
	}
	else
	{
		for (const auto &logItem : logArray)
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
