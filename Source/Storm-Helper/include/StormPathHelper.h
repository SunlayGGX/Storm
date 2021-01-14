#pragma once

#include "NonInstanciable.h"

#include <filesystem>


namespace Storm
{
	struct StormPathHelper : private Storm::NonInstanciable
	{
	public:
		static std::filesystem::path findStormRootPath(const std::filesystem::path &exeFolderPath);
	};
}
