#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	enum class ThreadFlagEnum : uint64_t;

	/*
	If any of modules is a dll, then this Flagger and all related implementation should be moved into its own separate dll to avoid dll duplicated flags
	Since a dll has its own set of global variable it doesn't share with each other.
	*/
	class ThreadingFlagger : private Storm::NonInstanciable
	{
	public:
		static void addThreadFlag(const Storm::ThreadFlagEnum threadFlag) noexcept;
	};
}
