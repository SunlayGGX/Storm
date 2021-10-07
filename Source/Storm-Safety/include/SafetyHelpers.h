#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	class SafetyHelpers : private Storm::NonInstanciable
	{
	public:
		static void sendCleanCloseTermination(const std::string_view msg);
		static void sendCleanExitTermination(const std::string_view msg);
		static void sendHardKillTermination(const std::string_view msg);
	};
}
