#include "ThreadManager.h"

#include "AsyncActionHolder.h"


#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#	include <Windows.h>
#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN

#include <processthreadsapi.h>
#include <comdef.h>


Storm::ThreadManager::ThreadManager() = default;
Storm::ThreadManager::~ThreadManager() = default;

void Storm::ThreadManager::nameCurrentThread(const std::wstring &newName)
{
	HRESULT res = ::SetThreadDescription(::GetCurrentThread(), newName.c_str());
	if (FAILED(res))
	{
		LOG_ERROR << "Cannot set the name of the current thread to '" << std::filesystem::path{ newName }.string() << "'. Reason " << std::filesystem::path{ _com_error{ res }.ErrorMessage() }.string();
	}
}
