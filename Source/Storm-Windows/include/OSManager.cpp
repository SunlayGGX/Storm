#include "OSManager.h"

#include "CDialogEventHandler.h"
#include "CoInitializerRAII.h"

#include "StormProcess.h"

#include "OSHelper.h"

#include "MemoryInfos.h"

#include "RAII.h"

#include <atlbase.h>
#include <comdef.h>
#include <Psapi.h>
#include <sysinfoapi.h>
#include <msi.h>
#include <set>


namespace
{
	std::unique_ptr<COMDLG_FILTERSPEC[]> makeFilters(const std::map<std::wstring, std::wstring> &filters, std::size_t &filtersCount)
	{
		std::unique_ptr<COMDLG_FILTERSPEC[]> filtersResult;

		filtersCount = filters.size();
		if (filtersCount > 0)
		{
			filtersResult = std::make_unique<COMDLG_FILTERSPEC[]>(filtersCount);
			std::size_t index = 0;
			for (const auto &filter : filters)
			{
				filtersResult[index] = { filter.first.c_str(), filter.second.c_str() };
			}
		}
		else
		{
			filtersCount = 1;
			filtersResult = std::make_unique<COMDLG_FILTERSPEC[]>(filtersCount);
			filtersResult[0] = { L"All Documents (*.*)", L"*.*" };
		}

		return filtersResult;
	}

	std::wstring retrieveFirstFilter(const std::map<std::wstring, std::wstring> &filters)
	{
		if (!filters.empty())
		{
			return std::begin(filters)->second;
		}
		else
		{
			return L"*.*";
		}
	}

	std::string generateComError(HRESULT result, const std::string_view &baseMessage)
	{
		return std::string{ baseMessage } + " Error was " + Storm::toStdString(_com_error{ result });
	}

	template<class Type>
	class Advisor
	{
	public:
		template<class EventType>
		Advisor(Type &ptr, EventType &eventHandlerVal, DWORD &cookie) :
			_ptr{ ptr },
			_cookie{ cookie }
		{
			ptr->Advise(eventHandlerVal, &cookie);
		}

		~Advisor()
		{
			if (_ptr != nullptr)
			{
				_ptr->Unadvise(_cookie);
			}
		}

	public:
		Type &_ptr;
		DWORD &_cookie;
	};

	void listInstalledSoftwareUsingRegKey(HKEY rootKey, const std::basic_string_view<TCHAR> sRoot, std::set<std::wstring> &inOutResults)
	{
		HKEY hUninstKey = nullptr;
		HKEY hAppKey = nullptr;
		TCHAR sAppKeyName[1024];
		TCHAR sSubAppKey[1024];
		TCHAR res[1024];
		long lResult = ERROR_SUCCESS;
		DWORD dwBufferSize = 0;

		//Open the "Uninstall" key.
		if (::RegOpenKeyEx(rootKey, sRoot.data(), 0, KEY_READ, &hUninstKey) != ERROR_SUCCESS)
		{
			LOG_ERROR << "Cannot open the uninstall key";
			return;
		}

		auto hUninstKeyCloser = Storm::makeLazyRAIIObject([&hUninstKey]()
		{
			RegCloseKey(hUninstKey);
		});

		for (DWORD dwIndex = 0; lResult != ERROR_NO_MORE_ITEMS; ++dwIndex)
		{
			//Enumerate all sub keys...
			dwBufferSize = sizeof(sAppKeyName) / sizeof(TCHAR);
			lResult = RegEnumKeyEx(hUninstKey, dwIndex, sAppKeyName, &dwBufferSize, nullptr, nullptr, nullptr, nullptr);

			if (::StrCmpC(sAppKeyName, TEXT("Classes")) == 0)
			{
				continue;
			}

			if (lResult == ERROR_SUCCESS)
			{
				::wsprintf(sSubAppKey, L"%s\\%s", sRoot.data(), sAppKeyName);
				if (::RegOpenKeyEx(rootKey, sSubAppKey, 0, KEY_READ, &hAppKey) != ERROR_SUCCESS)
				{
					continue;
				}

				auto hAppKeyCloser = Storm::makeLazyRAIIObject([&hAppKey]()
				{
					RegCloseKey(hAppKey);
				});

				long subResult = ERROR_SUCCESS;
				for (DWORD subIndex = 0; subResult != ERROR_NO_MORE_ITEMS; ++subIndex)
				{
					dwBufferSize = sizeof(res) / sizeof(TCHAR);
					subResult = ::RegEnumKeyEx(hAppKey, subIndex, res, &dwBufferSize, nullptr, nullptr, nullptr, nullptr);
					if(subResult == ERROR_SUCCESS)
					{
						auto valPair = inOutResults.emplace(Storm::toStdWString(sAppKeyName) + L' ' + Storm::toStdWString(res));
						if (valPair.second)
						{
							if (auto bracketFound = valPair.first->find('{'); bracketFound != std::string::npos)
							{
								if (auto clip = valPair.first->find('}', bracketFound); clip != std::string::npos)
								{
									inOutResults.erase(valPair.first);
								}
								else
								{
									auto node = inOutResults.extract(valPair.first);
									node.value().erase(bracketFound, clip - bracketFound);
									inOutResults.insert(std::move(node));
								}
							}
						}
					}
					else if (subResult == ERROR_NO_MORE_ITEMS)
					{
						break;
					}
				}
			}
			else if (lResult == ERROR_NO_MORE_ITEMS)
			{
				break;
			}
		}
	}
}

namespace Storm
{
	namespace details
	{
		class ProcessesHolder
		{
		private:
			template<class ProcessesContainer>
			static void prepareDestroyAll(ProcessesContainer &procArray)
			{
				for (auto &processPair : procArray)
				{
					processPair.second.prepareDestroy();
				}
			}

		public:
			ProcessesHolder() :
				_referralId{ 0 }
			{}

			~ProcessesHolder()
			{
				Storm::details::ProcessesHolder::prepareDestroyAll(_processes);
			}

		public:
			std::size_t startProcess(Storm::StormProcessStartup &&startup)
			{
				Storm::StormProcess tmp{ std::move(startup) };

				std::lock_guard<std::mutex> lock{ _processMutex };
				_processes.emplace(++_referralId, std::move(tmp));
				return _referralId;
			}

			void clearProcesses()
			{
				decltype(_processes) tmp;

				{
					std::lock_guard<std::mutex> lock{ _processMutex };
					std::swap(tmp, _processes);
				}

				Storm::details::ProcessesHolder::prepareDestroyAll(tmp);
			}

			int queryProcessExitCode(const std::size_t processUID, bool &outReturned, bool &outFailure) const
			{
				std::lock_guard<std::mutex> lock{ _processMutex };
				if (const auto found = _processes.find(processUID); found != std::end(_processes))
				{
					const Storm::StormProcess &proc = found->second;
					return proc.getExitCode(outReturned, outFailure);
				}
				else
				{
					Storm::throwException<Storm::Exception>("Cannot find the process " + Storm::toStdString(processUID) + " managed by this module");
				}
			}

			int waitForProcessExitCode(const std::size_t processUID, bool &outFailure)
			{
				std::lock_guard<std::mutex> lock{ _processMutex };
				if (const auto found = _processes.find(processUID); found != std::end(_processes))
				{
					Storm::StormProcess &proc = found->second;
					return proc.waitForCompletion(outFailure);
				}
				else
				{
					Storm::throwException<Storm::Exception>("Cannot find the process " + Storm::toStdString(processUID) + " managed by this module");
				}
			}

		private:
			std::map<std::size_t, Storm::StormProcess> _processes;
			std::size_t _referralId;
			mutable std::mutex _processMutex;
		};
	}
}

Storm::OSManager::OSManager() :
	_processHolder{ std::make_unique<Storm::details::ProcessesHolder>() },
	_currentProcessHandle{ ::GetCurrentProcess() }
{}

Storm::OSManager::~OSManager() = default;

void Storm::OSManager::cleanUp_Implementation()
{
	_processHolder.reset();
}

void Storm::OSManager::clearProcesses()
{
	_processHolder->clearProcesses();
}

std::wstring Storm::OSManager::openFileExplorerDialog(const std::wstring &defaultStartingPath, const std::map<std::wstring, std::wstring> &filters)
{
	// https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/bb776913(v=vs.85)?redirectedfrom=MSDN

	std::wstring resultSelectedFilePath;

	LOG_DEBUG << "Requesting to open file explorer dialog to select a file.";

	std::size_t filterCount;
	auto filtersCom = makeFilters(filters, filterCount);

	Storm::CoInitializerRAII coInitializer{ COINIT_MULTITHREADED };

	// CoCreate the File Open Dialog object.
	CComPtr<IFileDialog> fileDialog = nullptr;
	HRESULT hr = ::CoCreateInstance(
		CLSID_FileOpenDialog,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&fileDialog)
	);

	if (SUCCEEDED(hr))
	{
		// Create an event handling object, and hook it up to the dialog.
		CComPtr<IFileDialogEvents> fileDialogEvent = nullptr;
		hr = Storm::CDialogEventHandler::createInstance(IID_PPV_ARGS(&fileDialogEvent));

		if (SUCCEEDED(hr))
		{
			// Hook up the event handler.
			DWORD dwCookie;
			Advisor fileDialogAdvisor{ fileDialog, fileDialogEvent, dwCookie };
			if (SUCCEEDED(hr))
			{
				// Set the options on the dialog.
				DWORD dwFlags;

				// Before setting, always get the options first in order 
				// not to override existing options.
				hr = fileDialog->GetOptions(&dwFlags);
				if (SUCCEEDED(hr))
				{
					// In this case, get shell items only for file system items.
					hr = fileDialog->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);
					if (SUCCEEDED(hr))
					{
						// Set the file types to display only. 
						// Notice that this is a 1-based array.
						hr = fileDialog->SetFileTypes(static_cast<UINT>(filterCount), filtersCom.get());
						if (SUCCEEDED(hr))
						{
							// Set the selected file type index to the first filter.
							hr = fileDialog->SetFileTypeIndex(1);
							if (SUCCEEDED(hr))
							{
								// Set the default extension to be the first filter.
								hr = fileDialog->SetDefaultExtension(retrieveFirstFilter(filters).c_str());
								if (SUCCEEDED(hr))
								{
									if (!defaultStartingPath.empty())
									{
										if (std::filesystem::is_directory(defaultStartingPath))
										{
											// Create the start folder command.
											CComPtr<IShellItem> psiFolder;

											hr = ::SHCreateItemFromParsingName(defaultStartingPath.c_str(), NULL, IID_PPV_ARGS(&psiFolder));
											if (SUCCEEDED(hr))
											{
												hr = fileDialog->SetFolder(psiFolder);
												if (SUCCEEDED(hr))
												{
													LOG_DEBUG << "Successfully set the starting folder location to " << Storm::toStdString(defaultStartingPath);
												}
												else
												{
													LOG_ERROR << generateComError(hr,
														Storm::toStdString(defaultStartingPath) + " failed to be set as starting folder! We will use the default instead."
													);
												}
											}
											else
											{
												LOG_ERROR << generateComError(hr,
													"Cannot parse " + Storm::toStdString(defaultStartingPath) + " name to a IShellItem! Therefore we won't set it as the default folder."
												);
											}
										}
										else
										{
											LOG_ERROR << generateComError(hr,
												Storm::toStdString(defaultStartingPath) + " is not a valid directory! Therefore we won't set it as the starting folder and use default instead."
											);
										}
									}
									else
									{
										LOG_DEBUG << "defaultStartingPath is empty, therefore we will remain with the default explorer location.";
									}

									// Show the dialog
									hr = fileDialog->Show(NULL);
									if (SUCCEEDED(hr))
									{
										// Obtain the result once the user clicks 
										// the 'Open' button.
										// The result is an IShellItem object.
										CComPtr<IShellItem> shellItemResult;
										hr = fileDialog->GetResult(&shellItemResult);
										if (SUCCEEDED(hr))
										{
											PWSTR pszFilePath = nullptr;
											hr = shellItemResult->GetDisplayName(
												SIGDN_FILESYSPATH,
												&pszFilePath
											);
											if (SUCCEEDED(hr))
											{
												std::unique_ptr<WCHAR, decltype(&::CoTaskMemFree)> pszFilePathHolder{ pszFilePath, &::CoTaskMemFree };
												resultSelectedFilePath = pszFilePath;
											}
											else
											{
												LOG_ERROR << generateComError(hr, "Cannot grab the file dialog explorer result!");
											}
										}
										else
										{
											LOG_ERROR << generateComError(hr, "Cannot grab the file dialog result!");
										}
									}
									else
									{
										LOG_ERROR << generateComError(hr, "Cannot show the file dialog explorer!");
									}
								}
								else
								{
									LOG_ERROR << generateComError(hr, "Cannot set the default file extension in file dialog!");
								}
							}
							else
							{
								LOG_ERROR << generateComError(hr, "Cannot set the default file type index!");
							}
						}
						else
						{
							LOG_ERROR << generateComError(hr, "Cannot set file types for file dialog!");
						}
					}
					else
					{
						LOG_ERROR << generateComError(hr, "Cannot set file dialog option flags!");
					}
				}
				else
				{
					LOG_ERROR << generateComError(hr, "Cannot get file dialog option flags!");
				}
			}
			else
			{
				LOG_ERROR << generateComError(hr, "Cannot get file dialog option flags!");
			}
		}
		else
		{
			LOG_ERROR << generateComError(hr, "Cannot create dialog events!");
		}
	}
	else
	{
		LOG_ERROR << generateComError(hr, "Cannot create a file open dialog instance!");
	}

	return resultSelectedFilePath;
}

unsigned int Storm::OSManager::obtainCurrentPID() const
{
	return ::GetCurrentProcessId();
}


std::string Storm::OSManager::getComputerName() const
{
	TCHAR buffer[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD buffsize;
	if (::GetComputerName(buffer, &buffsize))
	{
		return Storm::toStdString(std::wstring_view{ buffer, buffer + buffsize });
	}
	else
	{
		LOG_DEBUG_ERROR <<
			"Cannot retrieve computer name from msdn method. Error was: " << GetLastError() << ".\n"
			"Falling back to environ.";
	}

	return Storm::OSHelper::getComputerNameFromEnviron();
}

std::size_t Storm::OSManager::startProcess(Storm::StormProcessStartup &&startup)
{
	return _processHolder->startProcess(std::move(startup));
}

int Storm::OSManager::queryProcessExitCode(const std::size_t processUID, bool &outReturned, bool &outFailure) const
{
	return _processHolder->queryProcessExitCode(processUID, outReturned, outFailure);
}

int Storm::OSManager::waitForProcessExitCode(const std::size_t processUID, bool &outFailure)
{
	return _processHolder->waitForProcessExitCode(processUID, outFailure);
}

void Storm::OSManager::makeBipSound(const std::chrono::milliseconds bipDuration) const
{
	enum : DWORD { k_frequencyHz = 523 };
	::Beep(k_frequencyHz, static_cast<DWORD>(bipDuration.count()));
}

std::size_t Storm::OSManager::retrieveCurrentAppUsedMemory() const
{
	::PROCESS_MEMORY_COUNTERS memCounter;
	if (::GetProcessMemoryInfo(static_cast<HANDLE>(_currentProcessHandle), &memCounter, sizeof(memCounter)))
	{
		return memCounter.WorkingSetSize;
	}
	else
	{
		LOG_DEBUG_ERROR << "Cannot query current process memory usage. Error code was " << GetLastError();
		return 0;
	}
}

bool Storm::OSManager::retrieveMemoryInfo(Storm::MemoryInfos &outMemoryInfos) const
{
	outMemoryInfos._usedMemory = this->retrieveCurrentAppUsedMemory();
	::MEMORYSTATUSEX memoryStatus;

	memoryStatus.dwLength = sizeof(memoryStatus);

	if (::GlobalMemoryStatusEx(&memoryStatus))
	{
		outMemoryInfos._availableMemory = memoryStatus.ullAvailPhys;
		outMemoryInfos._totalMemory = memoryStatus.ullTotalPhys;
		return true;
	}
	else
	{
		LOG_DEBUG_ERROR << "Cannot retrieve memory infos for the current workstation. Error code was " << GetLastError();

		outMemoryInfos._totalMemory = 0;
		outMemoryInfos._availableMemory = 0;
		return false;
	}
}

bool Storm::OSManager::preventShutdown()
{
	if (!::AbortSystemShutdown(nullptr))
	{
		LOG_DEBUG << "System shutdown prevented.";
		return true;
	}
	else
	{
		LOG_ERROR << generateComError(::GetLastError(), "We failed to prevent system shutdown.");
		return false;
	}
}

std::vector<std::wstring> Storm::OSManager::listAllInstalledSoftwares() const
{
	const auto toVect = [](std::set<std::wstring> &set)
	{
		return std::vector<std::wstring>{ std::make_move_iterator(std::begin(set)), std::make_move_iterator(std::end(set)) };
	};

	std::set<std::wstring> result;

	listInstalledSoftwareUsingRegKey(HKEY_CURRENT_USER, TEXT("SOFTWARE"), result);
	listInstalledSoftwareUsingRegKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE"), result);
	
	// https://docs.microsoft.com/en-us/windows/win32/api/msi/nf-msi-msienumproductsa
	TCHAR productCode[39];

	enum : std::basic_string<TCHAR>::size_type { k_initialSize = 256 };
	std::basic_string<TCHAR>::size_type initialSize = k_initialSize;
	std::basic_string<TCHAR> cachedDynamicData = std::basic_string<TCHAR>{ k_initialSize, TEXT('\0') };

	DWORD iter = 0;
	do
	{
		switch (::MsiEnumProducts(iter++, productCode))
		{
		case ERROR_BAD_CONFIGURATION:
			LOG_DEBUG_ERROR << "Configuration data is corrupted (MsiEnumProducts).";
			return toVect(result);

		case ERROR_INVALID_PARAMETER:
			LOG_DEBUG_ERROR << "Invalid parameter used inside MsiEnumProductsA.";
			return toVect(result);

		case ERROR_NO_MORE_ITEMS:
			return toVect(result);

		case ERROR_NOT_ENOUGH_MEMORY:
			LOG_ERROR << "Not enough memory to complete installed process enumeration.";
			return toVect(result);

		case ERROR_SUCCESS:
			break;
		
		default:
			__assume(false);
		}

		bool continueLoop = true;

		do
		{
			DWORD sz = static_cast<DWORD>(cachedDynamicData.size());
			switch (::MsiGetProductInfo(productCode, INSTALLPROPERTY_INSTALLEDPRODUCTNAME, cachedDynamicData.data(), &sz))
			{
			case ERROR_BAD_CONFIGURATION:
				LOG_DEBUG_ERROR << "Configuration data is corrupted (MsiGetProductInfo).";
				return toVect(result);

			case ERROR_INVALID_PARAMETER:
				LOG_DEBUG_ERROR << "Invalid parameter used inside MsiGetProductInfo.";
				return toVect(result);

			case ERROR_MORE_DATA:
				initialSize = std::max(static_cast<std::size_t>(sz), initialSize * 2);
				cachedDynamicData.resize(initialSize);
				break;

			case ERROR_NOT_ENOUGH_MEMORY:
				LOG_ERROR << "Not enough memory to complete installed process enumeration.";
				return toVect(result);

			case ERROR_SUCCESS:
				result.emplace(Storm::toStdWString(std::basic_string_view<TCHAR>{ cachedDynamicData.data(), sz }));
				continueLoop = false;
				break;

			case ERROR_UNKNOWN_PROPERTY:
				LOG_DEBUG_ERROR << "Requested property is unknown (MsiGetProductInfo).";

			case ERROR_UNKNOWN_PRODUCT:
				continueLoop = false;
				break;

			default:
				__assume(false);
			}
		} while (continueLoop);

	} while (true);
}
