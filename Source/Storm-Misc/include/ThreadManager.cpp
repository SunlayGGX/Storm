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
		LOG_ERROR << "Cannot set the name of the current thread to '" << std::filesystem::path{ newName }.string() << "'. Reason " << std::filesystem::path{ _com_error{ res }.ErrorMessage() }.string();
	}

	auto executor = std::make_unique<Storm::AsyncActionExecutor>();
	const auto currentThreadId = std::this_thread::get_id();

	std::lock_guard<std::mutex> lock{ _mutex };
	_toExecute[currentThreadId] = std::move(executor);
	_threadIdMapping[threadEnum] = currentThreadId;
	_threadEnumMapping[currentThreadId] = threadEnum;
}

void Storm::ThreadManager::executeOnThread(const std::thread::id &threadId, AsyncAction &&action)
{
	std::lock_guard<std::mutex> lock{ _mutex };
	this->executeOnThreadInternal(threadId, std::move(action));
}

void Storm::ThreadManager::executeOnThread(Storm::ThreadEnumeration threadEnum, Storm::AsyncAction &&action)
{
	std::lock_guard<std::mutex> lock{ _mutex };
	if (const auto found = _threadIdMapping.find(threadEnum); found != std::end(_threadIdMapping))
	{
		this->executeOnThreadInternal(found->second, std::move(action));
	}
	else
	{
		Storm::throwException<std::exception>("Requested thread was not registered to execute any callback!");
	}
}

void Storm::ThreadManager::processCurrentThreadActions()
{
	const auto thisThreadId = std::this_thread::get_id();

	std::lock_guard<std::mutex> lock{ _mutex };
	if (const auto executorFound = _toExecute.find(thisThreadId); executorFound != std::end(_toExecute))
	{
		executorFound->second->execute();
	}
	else
	{
		Storm::throwException<std::exception>(std::stringstream{} << "Thread with id " << thisThreadId << " was not registered to execute any callback!");
	}
}

bool Storm::ThreadManager::isExecutingOnThread(Storm::ThreadEnumeration threadEnum) const
{
	std::lock_guard<std::mutex> lock{ _mutex };
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
		Storm::throwException<std::exception>(std::stringstream{} << "Thread with id " << threadId << " was not registered to execute any callback!");
	}
}
