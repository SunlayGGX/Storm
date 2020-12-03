#include "ThreadManager.h"

#include "AsyncActionExecutor.h"

#include "ThrowException.h"

#include "ThreadPriority.h"

#include "LeanWindowsInclude.h"

#include <processthreadsapi.h>
#include <comdef.h>


namespace
{
	template<int priority>
	void setCurrentThreadPriorityImpl(const std::string_view &priorityName)
	{
		const HANDLE currentThreadHandle = ::GetCurrentThread();
		if (::GetThreadPriority(currentThreadHandle) != priority)
		{
			if (::SetThreadPriority(currentThreadHandle, priority))
			{
				LOG_DEBUG << "Current thread priority successfully changed to " << priorityName << " priority!";
			}
			else
			{
				LOG_ERROR << "We weren't able to set the current thread priority to " << priorityName << " priority!";
			}
		}
		else
		{
			LOG_DEBUG << "Current thread priority is already at " << priorityName << ". Therefore no change was made.";
		}
	}
}


Storm::ThreadManager::ThreadManager() = default;
Storm::ThreadManager::~ThreadManager() = default;

void Storm::ThreadManager::registerCurrentThread(Storm::ThreadEnumeration threadEnum, const std::wstring &newName)
{
	HRESULT res = ::SetThreadDescription(::GetCurrentThread(), newName.c_str());
	if (FAILED(res))
	{
		LOG_ERROR << "Cannot set the name of the current thread to '" << Storm::toStdString(newName) << "'. Reason " << Storm::toStdString(_com_error{ res });
	}

	auto executor = std::make_unique<Storm::AsyncActionExecutor>();
	const auto currentThreadId = std::this_thread::get_id();

	std::lock_guard<std::recursive_mutex> lock{ _mutex };
	_toExecute[currentThreadId] = std::move(executor);
	_threadIdMapping[threadEnum] = currentThreadId;
	_threadEnumMapping[currentThreadId] = threadEnum;

	// If there were pending actions for this thread. Transfer them now to the new executor that was just created.
	if (const auto pendingActionsIter = _pendingThreadsRegisteringActions.find(threadEnum); pendingActionsIter != std::end(_pendingThreadsRegisteringActions))
	{
		auto &executor = _toExecute[currentThreadId];
		for (Storm::AsyncAction &pendingAction : pendingActionsIter->second)
		{
			executor->bind(std::move(pendingAction));
		}

		_pendingThreadsRegisteringActions.erase(pendingActionsIter);
	}
}

void Storm::ThreadManager::executeOnThread(const std::thread::id &threadId, AsyncAction &&action)
{
	// Warning : You should never call a executeOnThread from a runParallel that is executed inside an AsyncAction being executed in processCurrentThreadActions.
	// If those 3 functions are present in your call stack, then a deadlock will occurs.
	std::lock_guard<std::recursive_mutex> lock{ _mutex };
	this->executeOnThreadInternal(threadId, std::move(action));
}

void Storm::ThreadManager::executeDefferedOnThread(Storm::ThreadEnumeration threadEnum, Storm::AsyncAction &&action)
{
	// Warning : You should never call a executeOnThread from a runParallel that is executed inside an AsyncAction being executed in processCurrentThreadActions.
	// If those 3 functions are present in your call stack, then a deadlock will occurs.

	const auto thisThreadId = std::this_thread::get_id();
	bool executeNow = false;

	std::lock_guard<std::recursive_mutex> lock{ _mutex };
	if (const auto found = _threadIdMapping.find(threadEnum); found != std::end(_threadIdMapping))
	{
		if (found->second != thisThreadId)
		{
			this->executeOnThreadInternal(found->second, std::move(action));
		}
		else
		{
			this->executeDefferedOnThreadInternal(found->second, std::move(action));
		}
	}
	else
	{
		_pendingThreadsRegisteringActions[threadEnum].emplace_back(std::move(action));
	}
}

void Storm::ThreadManager::executeOnThread(Storm::ThreadEnumeration threadEnum, Storm::AsyncAction &&action)
{// Warning : You should never call a executeOnThread from a runParallel that is executed inside an AsyncAction being executed in processCurrentThreadActions.
	// If those 3 functions are present in your call stack, then a deadlock will occurs.

	const auto thisThreadId = std::this_thread::get_id();
	bool executeNow = false;

	{
		std::lock_guard<std::recursive_mutex> lock{ _mutex };
		if (const auto found = _threadIdMapping.find(threadEnum); found != std::end(_threadIdMapping))
		{
			if (found->second != thisThreadId)
			{
				this->executeOnThreadInternal(found->second, std::move(action));
			}
			else
			{
				// If the current thread has requested to execute on itself... Do it right now.
				executeNow = true;
			}
		}
		else
		{
			_pendingThreadsRegisteringActions[threadEnum].emplace_back(std::move(action));
		}
	}

	if (executeNow)
	{
		Storm::AsyncActionExecutor tmpExecutor{};
		tmpExecutor.bind(std::move(action));
		tmpExecutor.execute();
	}
}

void Storm::ThreadManager::processCurrentThreadActions()
{
	const auto thisThreadId = std::this_thread::get_id();

	std::lock_guard<std::recursive_mutex> lock{ _mutex };
	if (const auto executorFound = _toExecute.find(thisThreadId); executorFound != std::end(_toExecute))
	{
		executorFound->second->execute();
	}
	else
	{
		Storm::throwException<std::exception>("Thread with id " + Storm::toStdString(thisThreadId) + " was not registered to execute any callback!");
	}
}

void Storm::ThreadManager::processActionsOfThread(Storm::ThreadEnumeration threadEnum)
{
	std::lock_guard<std::recursive_mutex> lock{ _mutex };
	if (const auto foundThreadId = _threadIdMapping.find(threadEnum); foundThreadId != std::end(_threadIdMapping))
	{
		if (const auto executorFound = _toExecute.find(foundThreadId->second); executorFound != std::end(_toExecute))
		{
			executorFound->second->execute();
		}
	}
	else
	{
		Storm::throwException<std::exception>("Thread with enumeration " + Storm::toStdString(threadEnum) + " was not registered to execute any callback!");
	}
}

bool Storm::ThreadManager::isExecutingOnThread(Storm::ThreadEnumeration threadEnum) const
{
	std::lock_guard<std::recursive_mutex> lock{ _mutex };
	if (const auto found = _threadIdMapping.find(threadEnum); found != std::end(_threadIdMapping))
	{
		return found->second == std::this_thread::get_id();
	}

	return false;
}

void Storm::ThreadManager::setCurrentThreadPriority(const Storm::ThreadPriority priority) const
{
	switch (priority)
	{
	case Storm::ThreadPriority::Below:
		setCurrentThreadPriorityImpl<THREAD_PRIORITY_BELOW_NORMAL>("below");
		break;

	case Storm::ThreadPriority::Normal:
		setCurrentThreadPriorityImpl<THREAD_PRIORITY_NORMAL>("normal");
		break;

	case Storm::ThreadPriority::High:
		setCurrentThreadPriorityImpl<THREAD_PRIORITY_HIGHEST>("high");
		break;

	case Storm::ThreadPriority::Unset:
	default:
		break;
	}
}

void Storm::ThreadManager::executeOnThreadInternal(const std::thread::id &threadId, AsyncAction &&action)
{
	if (const auto executorFound = _toExecute.find(threadId); executorFound != std::end(_toExecute))
	{
		executorFound->second->bind(std::move(action));
	}
	else
	{
		Storm::throwException<std::exception>("Thread with id " + Storm::toStdString(threadId) + " was not registered to execute any callback!");
	}
}

void Storm::ThreadManager::executeDefferedOnThreadInternal(const std::thread::id &threadId, Storm::AsyncAction &&action)
{
	if (const auto executorFound = _toExecute.find(threadId); executorFound != std::end(_toExecute))
	{
		executorFound->second->bindDeffered(std::move(action));
	}
	else
	{
		Storm::throwException<std::exception>("Thread with id " + Storm::toStdString(threadId) + " was not registered to execute any callback!");
	}
}
