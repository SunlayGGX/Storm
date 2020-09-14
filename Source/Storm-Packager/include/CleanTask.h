#pragma once

#include "ITaskLogic.h"


namespace StormPackager
{
	class CleanTask : public StormPackager::ITaskLogic
	{
	public:
		std::string_view getName() const final override;
		std::string prepare() final override;
		std::string execute() final override;
		std::string cleanUp() final override;
	};
}
