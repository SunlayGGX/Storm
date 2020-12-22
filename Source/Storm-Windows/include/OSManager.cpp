#include "OSManager.h"

#include "CDialogEventHandler.h"
#include "CoInitializerRAII.h"

#include <atlbase.h>
#include <comdef.h>


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
}

Storm::OSManager::OSManager() = default;
Storm::OSManager::~OSManager() = default;

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
	HRESULT hr = CoCreateInstance(
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

											hr = SHCreateItemFromParsingName(defaultStartingPath.c_str(), NULL, IID_PPV_ARGS(&psiFolder));
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
												std::unique_ptr<WCHAR, decltype(&CoTaskMemFree)> pszFilePathHolder{ pszFilePath, &CoTaskMemFree };
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
