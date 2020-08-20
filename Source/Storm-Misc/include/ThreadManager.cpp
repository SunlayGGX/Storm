#include "ThreadManager.h"

#include "AsyncActionExecutor.h"

#include "ThrowException.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#	include <Windows.h>
#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN

#include <processthreadsapi.h>
#include <comdef.h>


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
	std::lock_guard<std::recursive_mutex> lock{ _mutex };
	this->executeOnThreadInternal(threadId, std::move(action));
}

void Storm::ThreadManager::executeOnThread(Storm::ThreadEnumeration threadEnum, Storm::AsyncAction &&action)
{
	std::lock_guard<std::recursive_mutex> lock{ _mutex };
	if (const auto found = _threadIdMapping.find(threadEnum); found != std::end(_threadIdMapping))
	{
		this->executeOnThreadInternal(found->second, std::move(action));
	}
	else
	{
		_pendingThreadsRegisteringActions[threadEnum].emplace_back(std::move(action));
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

bool Storm::ThreadManager::isExecutingOnThread(Storm::ThreadEnumeration threadEnum) const
{
	std::lock_guard<std::recursive_mutex> lock{ _mutex };
	if (const auto found = _threadIdMapping.find(threadEnum); found != std::end(_threadIdMapping))
	{
		return found->second == std::this_thread::get_id();
	}

	return false;
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
