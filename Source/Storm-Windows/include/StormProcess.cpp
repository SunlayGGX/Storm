#include "StormProcess.h"

#include "StormProcessStartup.h"

#include "SingletonHolder.h"
#include "IThreadManager.h"
#include "ITimeManager.h"

#include "ThreadPriority.h"

#include "ThreadHelper.h"
#include "MemoryHelper.h"
#include "StringAlgo.h"

#include <shellapi.h>

#include <aclapi.h>

#include <locale>

#include <boost\algorithm\string\predicate.hpp>

#include <boost\asio\io_service.hpp>
#include <boost\asio\read.hpp>

#include <boost\process\child.hpp>
#include <boost\process\async_pipe.hpp>
#include <boost\process\io.hpp>
#include <boost\process\system.hpp>
#include <boost\process\async_system.hpp>


namespace Storm
{
	namespace details
	{
		namespace
		{
			using boost::process::pid_t;
		}

		class IStormProcessImpl
		{
		public:
			IStormProcessImpl(Storm::StormProcessStartup &&startupParam) :
				_exitCode{ std::numeric_limits<int64_t>::max() },
				_startupParam{ std::move(startupParam) }
			{}

			virtual ~IStormProcessImpl() = default;

		public:
			virtual void release() = 0;
			virtual void close() = 0;
			virtual int waitForCompletion(bool &outFailure) = 0;
			virtual void prepareDestroy() = 0;
			virtual bool isFailure() const = 0;

			bool hasExited() const noexcept
			{
				return _exitCode <= std::numeric_limits<int32_t>::max();;
			}

			int32_t getExitCode(bool &outHasExited, bool &outFailure) const noexcept
			{
				outHasExited = this->hasExited();
				outFailure = this->isFailure();
				return static_cast<int32_t>(_exitCode);
			}

			DWORD getProcessPID() const noexcept
			{
				return _processPID;
			}

			const std::string& getExePath() const noexcept
			{
				return _startupParam._exePath;
			}

		protected:
			int64_t _exitCode;
			pid_t _processPID;
			Storm::StormProcessStartup _startupParam;
		};
	}
}

namespace
{
	template<bool showWindow>
	int startElevated(const std::wstring &exeName, const std::wstring &commandlineArgs)
	{
		TCHAR verb[] = L"runas";

		SHELLEXECUTEINFO executeInfo;
		Storm::ZeroMemories(executeInfo);
		executeInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		executeInfo.hwnd = nullptr;
		executeInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		executeInfo.lpVerb = verb;
		executeInfo.lpFile = exeName.c_str();
		executeInfo.lpParameters = commandlineArgs.c_str();
		executeInfo.hInstApp = nullptr;

		if constexpr (showWindow)
		{
			executeInfo.nShow = SW_SHOWNORMAL;
		}
		else
		{
			executeInfo.nShow = SW_HIDE;
		}

		if (::ShellExecuteEx(&executeInfo))
		{
			DWORD exitCode;

			while (::GetExitCodeProcess(executeInfo.hProcess, &exitCode))
			{
				if (exitCode != STILL_ACTIVE)
				{
					return exitCode;
				}

				std::this_thread::sleep_for(std::chrono::milliseconds{ 200 });
			}

			return -98;
		}
		else
		{
			return -97;
		}
	}

	template<bool quote>
	std::string mountFinalCreateProcessCommand(const std::string &exePath, const std::string &args)
	{
		std::wstring result;

		const std::size_t commandLineSize = args.size();

		if constexpr (quote)
		{
			result.reserve(exePath.size() + commandLineSize + 4);
		}
		else
		{
			result.reserve(exePath.size() + commandLineSize + 2);
		}

		if constexpr (quote)
		{
			if (exePath[0] != '"')
			{
				result += '"';
			}
		}

		result += exePath;

		if constexpr (quote)
		{
			if (exePath.back() != '"')
			{
				result += '"';
			}
		}

		if (commandLineSize > 0)
		{
			result += ' ';
			result += args;
		}

		return result;
	}

	struct LogParserTraits
	{
	public:
		template<bool clearAfter> using StringType = std::conditional_t<clearAfter, std::string, const std::string>;
	};

	class ErrorLogPipeParser
	{
	protected:
		template<bool clearAfter>
		void parseMessage(LogParserTraits::StringType<clearAfter> &err, const Storm::StormProcessStartup &, const std::string &processPIDStr, const std::string &applicationNameStr)
		{
			LOG_ERROR <<
				"Error message received from process " << processPIDStr << " (" << applicationNameStr << ") :\n" <<
				err;

			if constexpr (clearAfter)
			{
				err.clear();
			}
		}
	};

	template<bool hasCallback>
	class OutputLogPipeParser
	{
	private:
		void logParseOutputMsg(const std::string &output, const std::string &processPIDStr, const std::string &applicationNameStr)
		{
			if (boost::icontains(output, "error", _defaultLocale))
			{
				LOG_ERROR <<
					"Error message received from process " << processPIDStr << " (" << applicationNameStr << ") :\n" <<
					output;
			}
			else if (boost::icontains(output, "warning", _defaultLocale))
			{
				LOG_WARNING <<
					"Warning message received from process " << processPIDStr << " (" << applicationNameStr << ") :\n" <<
					output;
			}
			else
			{
				LOG_DEBUG <<
					"Normal Message received from process " << processPIDStr << " (" << applicationNameStr << ") :\n" <<
					output;
			}
		};

	protected:
		template<bool clearAfter>
		void parseMessage(LogParserTraits::StringType<clearAfter> &output, const Storm::StormProcessStartup &startupParam, const std::string &processPIDStr, const std::string &applicationNameStr)
		{
			if (!output.empty())
			{
				if constexpr (hasCallback)
				{
					if (!startupParam._pipeCallback(output))
					{
						this->logParseOutputMsg(output, processPIDStr, applicationNameStr);
					}
				}
				else
				{
					this->logParseOutputMsg(output, processPIDStr, applicationNameStr);
				}

				if constexpr (clearAfter)
				{
					output.clear();
				}
			}
		}

	private:
		const std::locale _defaultLocale = std::locale();
	};

	template<class Child>
	class TaskkillProcess
	{
	private:
		friend Child;
		TaskkillProcess() = default;
		~TaskkillProcess() = default; // making it private is to prevent public inheritance to actually refer to Child through SoftKillProcess.

	protected:
		int close()
		{
			Child &current = static_cast<Child &>(*this);
			auto processPID = current.getProcessPID();

			std::string killProcessArg = "/pid " + std::to_string(processPID);

			int killResult = boost::process::system("taskkill " + killProcessArg);
			if (killResult == 1) // permission denied : start in elevated mode
			{
				LOG_DEBUG << "Taskkill permission denied, we need to restart it in elevated mode.";

				// taskkill actually soft kill an app. The soft kill is always a good kill because it waits before hard killing it.
				killResult = startElevated<false>(L"taskkill", Storm::toStdWString(std::move(killProcessArg)));
			}

			return killResult;
		}
	};


	template<class OutputPipeLogHandler, class ErrorPipeLogHandler, template<class Child> class ProcessKillerInjector>
	class NormalStormProcessImpl :
		public Storm::details::IStormProcessImpl,
		private ProcessKillerInjector<NormalStormProcessImpl<OutputPipeLogHandler, ErrorPipeLogHandler, ProcessKillerInjector>>,
		private OutputPipeLogHandler,
		private ErrorPipeLogHandler
	{
	private:
		using ParentKillerInjector = ProcessKillerInjector<NormalStormProcessImpl<OutputPipeLogHandler, ErrorPipeLogHandler, ProcessKillerInjector>>;

	public:
		NormalStormProcessImpl(Storm::StormProcessStartup &&startupParam) :
			Storm::details::IStormProcessImpl{ std::move(startupParam) },
			_outputPipe{ _ioService },
			_errorPipe{ _ioService },
			_running{ true }
		{
			this->start();
		}

		~NormalStormProcessImpl()
		{
			if (_startupParam._shareLife)
			{
				this->close();
			}

			Storm::join(_runningThread, _process);
		}

	private:
		void start()
		{
			_outputPipe = boost::process::async_pipe{ _ioService };
			_errorPipe = boost::process::async_pipe{ _ioService };

			_process = boost::process::child{
				_startupParam._exePath,
				_startupParam._commandLine,
				boost::process::std_out > _outputPipe,
				boost::process::std_err > _errorPipe
			};

			// Maybe useless, but in case process::child constructor doesn't throw on invalid params...
			_started = _process.running();

			_processPID = _process.id();

			_runningThread = std::thread{ [this, applicationNameStr = Storm::toStdString(_startupParam._exePath)] ()
			{
				this->run(applicationNameStr);
			} };
		}

	private:
		template<class PipeLogHandler, std::size_t bufferCount>
		void parseMsgTransaction(std::string &msg, const std::string &applicationNameStr, const std::string &processPIDStr, char(&bufferData)[bufferCount])
		{
			msg += bufferData;
			Storm::ZeroMemories(bufferData);

			if (!msg.empty())
			{
				const bool isComplete = msg.back() == '\n';
				if (isComplete)
				{
					Storm::StringAlgo::replaceAll(msg, '\n', Storm::StringAlgo::makeReplacePredicate("\r\n"));
					while (!msg.empty() && msg.back() == '\n')
					{
						msg.pop_back();
					}

					PipeLogHandler::parseMessage<true>(msg, _startupParam, processPIDStr, applicationNameStr);
				}
			}
		}

#pragma push_macro("STORM_MODULE_NAME")
#undef STORM_MODULE_NAME
#define STORM_MODULE_NAME "External"
		void run(const std::string &applicationNameStr)
		{
			const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

			{
				Storm::IThreadManager &threadMgr = singletonHolder.getSingleton<Storm::IThreadManager>();
				threadMgr.setCurrentThreadPriority(Storm::ThreadPriority::Below);
			}

			enum : std::size_t
			{
				k_bufferDefaultSize = 2048,
				k_bufferMaxUseSize = k_bufferDefaultSize - 1, // Always keep a '\0' constant at the end to prevent overflow.
			};

			char outBufferData[k_bufferDefaultSize];
			char errBufferData[k_bufferDefaultSize];

			Storm::ZeroMemories(outBufferData, errBufferData);

#define STORM_OUT_BUFFER boost::asio::buffer(outBufferData, k_bufferMaxUseSize)
#define STORM_ERR_BUFFER boost::asio::buffer(errBufferData, k_bufferMaxUseSize)

			const std::string processPIDStr = std::to_string(_processPID);

			auto readLambda = [this, &processPIDStr, &applicationNameStr](const boost::system::error_code &ec, const std::size_t)
			{
				if (ec && _running)
				{
					_running = false;
					LOG_ERROR << "Pipe for application " << processPIDStr << " (" << applicationNameStr << ") was broken!";
				}
			};

			boost::asio::async_read(_outputPipe, STORM_OUT_BUFFER, readLambda);
			boost::asio::async_read(_errorPipe, STORM_ERR_BUFFER, readLambda);

			Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();
			
			constexpr std::chrono::milliseconds refreshTime{ 300 };
			constexpr std::chrono::milliseconds ioServiceRunTime{ 10 };

			while (_running && _process.running() && timeMgr.waitForTimeOrExit(refreshTime))
			{
				_ioService.run_for(ioServiceRunTime);

				this->parseMsgTransaction<OutputPipeLogHandler>(_outputMsg, applicationNameStr, processPIDStr, outBufferData);
				this->parseMsgTransaction<ErrorPipeLogHandler>(_errMsg, applicationNameStr, processPIDStr, errBufferData);

				if (_running)
				{
					boost::asio::async_read(_outputPipe, STORM_OUT_BUFFER, readLambda);
					boost::asio::async_read(_errorPipe, STORM_ERR_BUFFER, readLambda);
				}
			}

			_outputMsg += outBufferData;
			if (!_outputMsg.empty())
			{
				OutputPipeLogHandler::parseMessage<true>(_outputMsg, _startupParam, processPIDStr, applicationNameStr);
			}

			_errMsg += errBufferData;
			if (!_errMsg.empty())
			{
				ErrorPipeLogHandler::parseMessage<true>(_errMsg, _startupParam, processPIDStr, applicationNameStr);
			}
		}
#undef STORM_OUT_BUFFER
#undef STORM_ERR_BUFFER


#undef STORM_MODULE_NAME
#pragma pop_macro("STORM_MODULE_NAME")

	public:
		void prepareDestroy() final override
		{
			_running = false;

			if (_startupParam._shareLife)
			{
				this->close();
			}
			else
			{
				this->release();
			}
		}

		void release() final override
		{
			if (!this->hasExited())
			{
				Storm::detach(_process);
			}
		}

		void close() final override
		{
			if (this->hasExited())
			{
				return;
			}
			else if (_process.running())
			{
				const std::string processPIDStr = Storm::toStdString(this->getProcessPID());

				// Boost does direct hard kill. This is not what we expect because the other process could be moving/copying/writing/... files. Or manipulating drivers,
				// Therefore we need to soft-kill them before hard-killing them.
				int killCode = ParentKillerInjector::close();
				if (killCode == S_OK)
				{
					LOG_DEBUG << "Successfully killed application with pid " << processPIDStr << " (" << _startupParam._exePath << ").";
				}
				else if (_process.running())
				{
					LOG_DEBUG << "We cannot kill application with pid " << processPIDStr << " (" << _startupParam._exePath << "). Kill code : " << killCode;
					_process.terminate();
				}
			}

			_exitCode = static_cast<int64_t>(_process.exit_code());
		}

		int waitForCompletion(bool &outFailure) final override
		{
			if (_started)
			{
				outFailure = false;
				constexpr std::chrono::milliseconds waitTime{ 200 };
				const Storm::ITimeManager &timeMgr = Storm::SingletonHolder::instance().getSingleton<Storm::ITimeManager>();
				while (timeMgr.isRunning())
				{
					if (_process.wait_for(waitTime))
					{
						int exitCode = _process.exit_code();
						_exitCode = static_cast<int64_t>(exitCode);
						return exitCode;
					}
				}

				LOG_DEBUG_WARNING << "Wait aborted because we stopped it.";
			}
			else
			{
				outFailure = true;
			}
			return 0;
		}

		bool isFailure() const final override
		{
			return !_started;
		}

	private:
		boost::process::child _process;

		boost::asio::io_service _ioService;
		boost::process::async_pipe _outputPipe;
		boost::process::async_pipe _errorPipe;

		std::string _outputMsg;
		std::string _errMsg;

		bool _started;

		std::thread _runningThread;
		std::atomic<bool> _running;
	};

	// Not even pipes
	class MinimalCmdStormProcessImpl :
		public Storm::details::IStormProcessImpl
	{
	public:
		MinimalCmdStormProcessImpl(Storm::StormProcessStartup &&startupParam) :
			Storm::details::IStormProcessImpl{ std::move(startupParam) },
			_running{ true },
			_failure{ false }
		{
			this->init();
		}

		~MinimalCmdStormProcessImpl()
		{
			Storm::join(_thread);
		}

	private:
		void init()
		{
			const std::size_t exePathLength = _startupParam._exePath.size();
			const std::size_t commandLineLength = _startupParam._commandLine.size();

			_finalArg.reserve(std::max(exePathLength, static_cast<std::size_t>(7)) + commandLineLength + 1);

			if (exePathLength > 0)
			{
				_finalArg += _startupParam._exePath;
			}
			else
			{
				_finalArg += "cmd /C";
			}

			if (commandLineLength > 0)
			{
				_finalArg += ' ';
				_finalArg += _startupParam._commandLine;
			}

			boost::process::async_system(_ioService, std::bind(&MinimalCmdStormProcessImpl::finishedCallback, this, std::placeholders::_1, std::placeholders::_2), _finalArg);

			_thread = std::thread{ [this]()
			{
				const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

				{
					Storm::IThreadManager &threadMgr = singletonHolder.getSingleton<Storm::IThreadManager>();
					threadMgr.setCurrentThreadPriority(Storm::ThreadPriority::Below);
				}

				Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();

				constexpr std::chrono::milliseconds refreshTime{ 300 };
				constexpr std::chrono::milliseconds ioServiceRunTime{ 2 };

				// The io service is only executed in this thread, so the only callback that modifies _running.
				// therefore _running is only modified in this thread, so it is safe to query it without locking 
				// in another word : we cannot read here and write at the same time because the only writer is also the reader.
				while (_running && timeMgr.waitForTimeOrExit(refreshTime))
				{
					_ioService.run_for(ioServiceRunTime);
				}
			} };

			// Wait a little the time the thread really started and 
			std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });

			// check further because command line are tricky, it return it has succeeded but in reality, it does not because it was executed asynchronously.
			// This check is so-so but I still searching for a better fix...
			std::unique_lock<std::mutex> lock{ _mutex };
			if (_failure)
			{
				_running = false;

				lock.unlock();
				Storm::join(_thread);

				Storm::throwException<Storm::Exception>("The cmd command returned with an error and so it has not started!");
			}
		}

	public:
		void prepareDestroy() final override
		{
			// Nothing to do since we don't own the process (with boost)...
			// So just prepare to leave the thread.
			std::lock_guard<std::mutex> lock{ _mutex };
			_running = false;
		}

		void release() final override
		{
			// Cmd process monitored by boost are released by default (detached)...
		}

		void close() final override
		{
			// Cmd process monitored by boost cannot be closed...
			// And even if we do, closing the cmd won't have an impact on processes it launched before being stopped.
		}

		int waitForCompletion(bool &outFailure) final override
		{
			{
				std::lock_guard<std::mutex> lock{ _mutex };
				outFailure = _failure;
				if (!_running)
				{
					return static_cast<int>(_exitCode);
				}
			}

			constexpr std::chrono::milliseconds waitTime{ 50 };
			Storm::ITimeManager &timeMgr = Storm::SingletonHolder::instance().getSingleton<Storm::ITimeManager>();
			while (timeMgr.waitForTimeOrExit(waitTime))
			{
				std::lock_guard<std::mutex> lock{ _mutex };
				if (!_running)
				{
					return static_cast<int>(_exitCode);
				}
			}

			LOG_DEBUG_WARNING << "Wait aborted because we stopped it.";
			return 0;
		}

	private:
		void finishedCallback(const boost::system::error_code &ec, int exitCode)
		{
			std::unique_lock<std::mutex> lock{ _mutex };
			_exitCode = static_cast<int64_t>(exitCode);
			_running = false;
			_ioService.stop();

			if (!ec)
			{
				_failure = true;
				
				lock.unlock();

				LOG_ERROR << "An error occurred with Cmd process we started to execute '" << _finalArg << "'";
			}
		}

		bool isFailure() const final override
		{
			std::lock_guard<std::mutex> lock{ _mutex };
			return _failure;
		}

	private:
		boost::asio::io_service _ioService;

		std::string _finalArg;

		mutable std::mutex _mutex;
		std::thread _thread;
		bool _running;
		bool _failure;
	};

	std::unique_ptr<Storm::details::IStormProcessImpl> createProcessObject(Storm::StormProcessStartup &&startupParam)
	{
		if (startupParam._exePath.empty() && !startupParam._isCmd)
		{
			Storm::throwException<Storm::Exception>("Exe path of the process to start shouldn't be empty!");
		}

		if ((startupParam._pipeCallback != nullptr || startupParam._bindIO) && startupParam._isCmd)
		{
			Storm::throwException<Storm::Exception>("Bind IO with command prompt start is not supported (maybe later).");
		}

		if (startupParam._pipeCallback == nullptr)
		{
			if (startupParam._isCmd)
			{
				return std::make_unique<MinimalCmdStormProcessImpl>(std::move(startupParam));
			}
			else
			{
				return std::make_unique<NormalStormProcessImpl<OutputLogPipeParser<false>, ErrorLogPipeParser, TaskkillProcess>>(std::move(startupParam));
			}
		}
		else
		{
			return std::make_unique<NormalStormProcessImpl<OutputLogPipeParser<true>, ErrorLogPipeParser, TaskkillProcess>>(std::move(startupParam));
		}
	}
}


Storm::StormProcess::StormProcess(Storm::StormProcessStartup &&startupParam) :
	_processImpl{ createProcessObject(std::move(startupParam)) }
{

}

Storm::StormProcess::StormProcess(Storm::StormProcess &&other) = default;
Storm::StormProcess::~StormProcess() = default;

void Storm::StormProcess::prepareDestroy()
{
	_processImpl->prepareDestroy();
}

void Storm::StormProcess::release()
{
	_processImpl->release();
}

void Storm::StormProcess::close()
{
	_processImpl->close();
}

int Storm::StormProcess::waitForCompletion(bool &outFailure)
{
	return _processImpl->waitForCompletion(outFailure);
}

int32_t Storm::StormProcess::getExitCode(bool &outHasExited, bool &outFailure) const
{
	return _processImpl->getExitCode(outHasExited, outFailure);
}
