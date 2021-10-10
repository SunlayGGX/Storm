#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	class VisualStudioOutputCompliantParser : Storm::NonInstanciable
	{
	public:
		static void produceVSOutputCompliantLine(std::string &inOutAppend, const std::string_view filePath, const std::size_t line);
		static void produceVSOutputCompliantLine(std::string &inOutAppend, const std::size_t preNumber, const std::string_view filePath, const std::size_t line);
	};
}
