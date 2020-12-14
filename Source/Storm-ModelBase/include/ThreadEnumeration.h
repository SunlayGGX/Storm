#pragma once


namespace Storm
{
	enum class ThreadEnumeration
	{
		Any,

		MainThread,
		GraphicsThread,
		LoggerThread,
		TimeThread,
		WindowsAndInputThread,
		SerializerThread,
		ScriptThread,
	};
}
