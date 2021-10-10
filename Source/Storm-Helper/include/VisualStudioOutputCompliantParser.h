#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	class VisualStudioOutputCompliantParser : Storm::NonInstanciable
	{
	public:
		// Returns if the path has been appended as Visual Studio Output expects.
		static bool produceVSOutputCompliantLine(std::string &inOutAppend, const std::string_view filePath, const std::size_t line);
		static bool produceVSOutputCompliantLine(std::string &inOutAppend, const std::size_t preNumber, const std::string_view filePath, const std::size_t line);
	};
}
