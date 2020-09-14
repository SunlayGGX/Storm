#pragma once

#include "ITaskLogic.h"

#include <set>


namespace StormPackager
{
	class CopierTask : public StormPackager::ITaskLogic
	{
	public:
		std::string_view getName() const final override;
		std::string prepare() final override;
		std::string execute() final override;
		std::string cleanUp() final override;

	private:
		std::set<std::filesystem::path> _foldersToCreate;
		std::vector<std::pair<std::filesystem::path, std::filesystem::path>> _toCopy;
	};
}
