#include "MFCHelper.h"

#include "LeanWindowsInclude.h"

#define APSTUDIO_INVOKED
#	include "resource.h"
#undef APSTUDIO_INVOKED

namespace
{
	static UINT g_cmdIdIncreased = _APS_NEXT_COMMAND_VALUE + 10000;
}


HMENU Storm::MFCHelper::findMenuById(HMENU mainMenu, const UINT menuID)
{
	MENUITEMINFO childinfo;
	childinfo.cbSize = sizeof(childinfo);
	childinfo.fMask = MIIM_SUBMENU;

	if (!::GetMenuItemInfo(mainMenu, menuID, FALSE, &childinfo))
	{
		LOG_DEBUG_ERROR << "Cannot find menu item with id " << menuID << ". Error code was " << GetLastError();
	}

	return childinfo.hSubMenu;
}

HMENU Storm::MFCHelper::findMenuByName(HMENU current, const std::wstring_view &menuName)
{
	int subItemCount = ::GetMenuItemCount(current);
	if (subItemCount == -1)
	{
		return nullptr;
	}

	for (int iter = 0; iter < subItemCount; ++iter)
	{
		MENUITEMINFO childinfo;
		childinfo.cbSize = sizeof(childinfo);
		childinfo.fMask = MIIM_STRING | MIIM_SUBMENU;
		childinfo.dwTypeData = nullptr;

		if (::GetMenuItemInfo(current, iter, TRUE, &childinfo))
		{
			if (childinfo.cch == menuName.size())
			{
				std::wstring iterMenuName;
				iterMenuName.resize(childinfo.cch);
				++childinfo.cch;

				childinfo.dwTypeData = iterMenuName.data();
				if (::GetMenuItemInfo(current, iter, TRUE, &childinfo))
				{
					if (menuName == iterMenuName)
					{
						return current;
					}
				}
			}

			if (childinfo.hSubMenu)
			{
				HMENU recursiveFound = Storm::MFCHelper::findMenuByName(childinfo.hSubMenu, menuName);
				if (recursiveFound != nullptr)
				{
					return recursiveFound;
				}
			}
		}
	}

	return nullptr;
}

void Storm::MFCHelper::setMenuEnabled(HMENU mainMenu, const UINT menuID, bool enabled)
{
	if (::EnableMenuItem(mainMenu, menuID, MF_BYCOMMAND | (enabled ? MF_ENABLED : (MF_DISABLED | MF_GRAYED))) == -1)
	{
		LOG_DEBUG_ERROR << "Menu item with id " << menuID << " doesn't exist! We cannot " << (enabled ? "enable" : "disable") << " it.";
	}
}

bool Storm::MFCHelper::setMenuAsPopupMenu(HMENU mainMenu, const UINT menuID, HMENU newPopup)
{
	MENUITEMINFO childinfo;
	childinfo.cbSize = sizeof(childinfo);

	childinfo.fMask = MIIM_SUBMENU;
	childinfo.hSubMenu = newPopup;

	if (::SetMenuItemInfo(mainMenu, menuID, FALSE, &childinfo))
	{

	}

	return true;
}

bool Storm::MFCHelper::appendNewStringMenu(HMENU parent, const std::wstring &menuName, UINT &outNewMenuCommandID)
{
	outNewMenuCommandID = g_cmdIdIncreased++;

	if (::AppendMenu(parent, MF_ENABLED | MF_STRING, outNewMenuCommandID, menuName.c_str()))
	{
		return true;
	}

	LOG_DEBUG_ERROR << "We weren't able to create a new submenu! Error code was " << GetLastError();
	return false;
}
