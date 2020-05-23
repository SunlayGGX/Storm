#pragma once


namespace Storm
{
	using QuitDelegate = std::function<void()>;
	using FinishedInitializeDelegate = std::function<void(void*, bool)>;
}
