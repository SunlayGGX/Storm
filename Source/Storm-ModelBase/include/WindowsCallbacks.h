#pragma once


namespace Storm
{
	using QuitDelegate = std::function<void()>;
	using FinishedInitializeDelegate = std::function<void(void*, bool)>;
	using WindowsResizedDelegate = std::function<void(int, int)>;
	using WindowsMovedDelegate = std::function<void(int, int)>;
}
