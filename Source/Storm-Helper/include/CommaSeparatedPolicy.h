#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	struct CommaSeparatedPolicy : private Storm::NonInstanciable
	{
	public:
		constexpr static const char separator() noexcept
		{
			return ',';
		}
	};

	struct CommaSpaceSeparatedPolicy : private Storm::NonInstanciable
	{
	public:
		constexpr static const std::string_view separator() noexcept
		{
			return ", ";
		}
	};
}
