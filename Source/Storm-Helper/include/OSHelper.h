#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	class OSHelper : private Storm::NonInstanciable
	{
	public:
		static std::string getRawQuotedCommandline();
	};
}
