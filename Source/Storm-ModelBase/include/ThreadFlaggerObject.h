#pragma once


#include "ThreadFlagEnum.h"


namespace Storm
{
	class ThreadFlaggerObject
	{
	public:
		const Storm::ThreadFlaggerObject& operator<<(const Storm::ThreadFlagEnum flag) const;
	};
}

#define STORM_DECLARE_THIS_THREAD_IS Storm::ThreadFlaggerObject{}
