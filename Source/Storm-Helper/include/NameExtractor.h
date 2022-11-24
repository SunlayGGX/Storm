#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	struct NameExtractor : private Storm::NonInstanciable
	{
	public:
		// Not from method... like it doesn't work for __FUNCTION__ because I'm lazy to make an algorithm for something I don't need. YAGNI...
		static consteval std::string_view extractTypeNameFromType(const std::string_view &fullName)
		{
			std::size_t posLastDot = fullName.find_last_of(':');
			return fullName.substr(posLastDot == std::string_view::npos ? 0 : posLastDot + 1);
		}
	};
}
