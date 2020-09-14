#pragma once

#include "NonInstanciable.h"


namespace StormPackager
{
	class PackagerHelper : private Storm::NonInstanciable
	{
	public:
		static bool createDirectory(const std::filesystem::path &toCreate);
		static bool copy(const std::filesystem::path &from, const std::filesystem::path &to);
		static bool erase(const std::filesystem::path &toRemove);
	};
}
