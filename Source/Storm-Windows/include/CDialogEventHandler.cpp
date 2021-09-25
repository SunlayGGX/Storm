#include "CDialogEventHandler.h"

#include <objbase.h>
#include <shtypes.h>
#include <shlwapi.h>

IFACEMETHODIMP Storm::CDialogEventHandler::QueryInterface(REFIID riid, void** ppv)
{
	static const QITAB qit[] = {
			QITABENT(CDialogEventHandler, IFileDialogEvents),
			QITABENT(CDialogEventHandler, IFileDialogControlEvents),
			{ 0 },
	};
	return QISearch(this, qit, riid, ppv);
}

IFACEMETHODIMP_(ULONG) Storm::CDialogEventHandler::AddRef()
{
	return InterlockedIncrement(&_cRef);
}

IFACEMETHODIMP_(ULONG) Storm::CDialogEventHandler::Release()
{
	const long cRef = InterlockedDecrement(&_cRef);
	if (!cRef)
	{
		delete this;
	}
	return cRef;
}

IFACEMETHODIMP Storm::CDialogEventHandler::OnFileOk(IFileDialog*)
{
	return S_OK;
}

IFACEMETHODIMP Storm::CDialogEventHandler::OnFolderChange(IFileDialog*)
{
	return S_OK;
}

IFACEMETHODIMP Storm::CDialogEventHandler::OnFolderChanging(IFileDialog*, IShellItem*)
{
	return S_OK;
}

IFACEMETHODIMP Storm::CDialogEventHandler::OnHelp(IFileDialog*)
{
	return S_OK;
}

IFACEMETHODIMP Storm::CDialogEventHandler::OnSelectionChange(IFileDialog*)
{
	return S_OK;
}

IFACEMETHODIMP Storm::CDialogEventHandler::OnShareViolation(IFileDialog*, IShellItem*, FDE_SHAREVIOLATION_RESPONSE*)
{
	return S_OK;
}

IFACEMETHODIMP Storm::CDialogEventHandler::OnTypeChange(IFileDialog* pfd)
{
	return S_OK;
}

IFACEMETHODIMP Storm::CDialogEventHandler::OnOverwrite(IFileDialog*, IShellItem*, FDE_OVERWRITE_RESPONSE*)
{
	return S_OK;
}

IFACEMETHODIMP Storm::CDialogEventHandler::OnItemSelected(IFileDialogCustomize* pfdc, DWORD dwIDCtl, DWORD dwIDItem)
{
	return S_OK;
}

IFACEMETHODIMP Storm::CDialogEventHandler::OnButtonClicked(IFileDialogCustomize*, DWORD)
{
	return S_OK;
}

IFACEMETHODIMP Storm::CDialogEventHandler::OnCheckButtonToggled(IFileDialogCustomize*, DWORD, BOOL)
{
	return S_OK;
}

IFACEMETHODIMP Storm::CDialogEventHandler::OnControlActivating(IFileDialogCustomize*, DWORD)
{
	return S_OK;
}

HRESULT Storm::CDialogEventHandler::createInstance(REFIID riid, void** ppv)
{
	*ppv = nullptr;
	CDialogEventHandler* dialogEventHandler = new (std::nothrow) CDialogEventHandler();
	HRESULT hr = dialogEventHandler ? S_OK : E_OUTOFMEMORY;
	if (SUCCEEDED(hr))
	{
		hr = dialogEventHandler->QueryInterface(riid, ppv);
		dialogEventHandler->Release();
	}
	return hr;
}
