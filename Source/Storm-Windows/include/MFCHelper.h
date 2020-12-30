#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	class MFCHelper : private Storm::NonInstanciable
	{
	public:
		static HMENU findMenuById(HMENU mainMenu, const UINT menuID);
		static HMENU findMenuByName(HMENU current, const std::wstring_view &menuName);
		static void setMenuEnabled(HMENU mainMenu, const UINT menuID, bool enabled);
		static bool setMenuAsPopupMenu(HMENU mainMenu, const UINT menuID, HMENU newPopup);
		static bool appendNewStringMenu(HMENU parent, const std::wstring &menuName, UINT &outNewMenuCommandID);
	};
}
