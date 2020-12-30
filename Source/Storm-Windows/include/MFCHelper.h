#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	class MFCHelper : private Storm::NonInstanciable
	{
	public:
		static HMENU findMenuById(const HMENU mainMenu, const UINT menuID);
		static HMENU findMenuByName(const HMENU current, const std::wstring_view &menuName);
		static void setMenuEnabled(const HMENU mainMenu, const UINT menuID, bool enabled);
		static bool appendNewStringMenu(const HMENU parent, const std::wstring &menuName, UINT &outNewMenuCommandID);
		static HMENU getChild(const HMENU parent, int childIter);
		static HMENU getFirstChild(const HMENU parent);
	};
}
