#pragma once

#include "IPackagingLogic.h"


namespace StormPackager
{
	class ValidaterPackager : public StormPackager::IPackagingLogic
	{
	public:
		std::string_view getName() const final override;
		std::string prepare() final override;
		std::string execute() final override;
		std::string cleanUp() final override;
	};
}
