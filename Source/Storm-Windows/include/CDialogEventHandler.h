#pragma once

#include <shlobj.h>
#include <shobjidl.h>

namespace Storm
{
	class CDialogEventHandler :
		public IFileDialogEvents,
		public IFileDialogControlEvents
	{
	private:
		constexpr CDialogEventHandler() = default;
		~CDialogEventHandler() = default;

	public:
		// IUnknown methods
		IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv);
		IFACEMETHODIMP_(ULONG) AddRef();
		IFACEMETHODIMP_(ULONG) Release();

		// IFileDialogEvents methods
		IFACEMETHODIMP OnFileOk(IFileDialog*);
		IFACEMETHODIMP OnFolderChange(IFileDialog*);
		IFACEMETHODIMP OnFolderChanging(IFileDialog*, IShellItem*);
		IFACEMETHODIMP OnHelp(IFileDialog*);
		IFACEMETHODIMP OnSelectionChange(IFileDialog*);
		IFACEMETHODIMP OnShareViolation(IFileDialog*, IShellItem*, FDE_SHAREVIOLATION_RESPONSE*);
		IFACEMETHODIMP OnTypeChange(IFileDialog*pfd);
		IFACEMETHODIMP OnOverwrite(IFileDialog*, IShellItem*, FDE_OVERWRITE_RESPONSE*);

		// IFileDialogControlEvents methods
		IFACEMETHODIMP OnItemSelected(IFileDialogCustomize* pfdc, DWORD dwIDCtl, DWORD dwIDItem);
		IFACEMETHODIMP OnButtonClicked(IFileDialogCustomize*, DWORD);
		IFACEMETHODIMP OnCheckButtonToggled(IFileDialogCustomize*, DWORD, BOOL);
		IFACEMETHODIMP OnControlActivating(IFileDialogCustomize*, DWORD);

		static HRESULT createInstance(REFIID riid, void** ppv);

	private:
		long _cRef = 1;
	};
}
