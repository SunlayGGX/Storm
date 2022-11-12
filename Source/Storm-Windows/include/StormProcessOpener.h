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
			bool _reset;

			// The meaning depends on the method that uses it.
			// Beware that it shouldn't be a temporary variable that is assigned to it (a string_view doesn't have ownership of the data).
			std::string_view _additionalParameterStr;
		};

	public:
		static bool openStormScriptSender(const Storm::StormProcessOpener::OpenParameter &param, std::size_t &outProcessUID);
		static bool openStormLogViewer(const Storm::StormProcessOpener::OpenParameter &param, std::size_t &outProcessUID);
		static bool openStormMaterialAvailability(const Storm::StormProcessOpener::OpenParameter &param, std::size_t &outProcessUID);
		static bool openRuntimeScript(const Storm::StormProcessOpener::OpenParameter &param, std::size_t &outProcessUID);
		static bool openCurrentConfigFile(const Storm::StormProcessOpener::OpenParameter &param, std::size_t &outProcessUID);
		static bool openReadmeFile(const Storm::StormProcessOpener::OpenParameter &param, std::size_t &outProcessUID);
		static bool openTextFile(const Storm::StormProcessOpener::OpenParameter &param, std::size_t &outProcessUID);
		static bool openStormUrlLink(const Storm::StormProcessOpener::OpenParameter &param, std::size_t &outProcessUID);
		static bool openStormRestarter(const Storm::StormProcessOpener::OpenParameter &param, std::size_t &outProcessUID);
		static bool openStormRootExplorer(const Storm::StormProcessOpener::OpenParameter &param, std::size_t &outProcessUID);
	};
}
