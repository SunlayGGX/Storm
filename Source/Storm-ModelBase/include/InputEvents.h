#pragma once


namespace Storm
{
	using KeyBinding = std::function<void()>;
	using MouseBinding = std::function<void(int, int, int, int)>;
	using WheelBinding = std::function<void(int)>;
}
