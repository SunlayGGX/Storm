#pragma once


namespace Storm
{
	enum class ThreadPriority
	{
		Unset, // Let the OS decide with its default settings.

		Below,
		Normal,
		High,

		// I don't expose Real time priority on purpose, unless you want to kill your workstation and make it totally unresponsive. 
	};
}
