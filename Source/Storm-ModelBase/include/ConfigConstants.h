#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	class ConfigConstants : private Storm::NonInstanciable
	{
	public:
		// Safety
		enum class SafetyConstants : long long
		{
			k_safetyThreadRefreshRateSeconds = 30
		};
	};
}
