#pragma once

#include "NonInstanciable.h"

namespace Storm
{
	class CSVHelpers : private Storm::NonInstanciable
	{
	public:
		static std::string transcriptLetterPosition(std::size_t position);
	};
}
