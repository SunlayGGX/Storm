#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	class StormProcessOpener : private Storm::NonInstanciable
	{
	public:
		struct OpenParameter
		{
		public:
			bool _failureQuit;
		};

	public:
		static bool openStormLogViewer(const Storm::StormProcessOpener::OpenParameter &param, std::size_t &outProcessUID);
		static bool openRuntimeScript(const Storm::StormProcessOpener::OpenParameter &param, std::size_t &outProcessUID);
		static bool openCurrentConfigFile(const Storm::StormProcessOpener::OpenParameter &param, std::size_t &outProcessUID);
		static bool openStormUrlLink(const Storm::StormProcessOpener::OpenParameter &param, std::size_t &outProcessUID, const std::string_view &url);
		static bool openStormGithubLink(const Storm::StormProcessOpener::OpenParameter &param, std::size_t &outProcessUID);
		static bool openStormRestarter(const Storm::StormProcessOpener::OpenParameter &param, std::size_t &outProcessUID);
		static bool openStormRootExplorer(const Storm::StormProcessOpener::OpenParameter &param, std::size_t &outProcessUID);
	};
}
