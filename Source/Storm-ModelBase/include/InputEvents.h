#pragma once


namespace Storm
{
	using KeyBinding = std::function<void()>;
	using MouseBinding = std::function<void()>;
	using WheelBinding = std::function<void(int)>;
}
