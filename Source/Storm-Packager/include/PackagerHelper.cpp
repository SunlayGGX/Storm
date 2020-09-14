#include "PackagerHelper.h"


bool StormPackager::PackagerHelper::createDirectory(const std::filesystem::path &toCreate)
{
	if (!std::filesystem::is_directory(toCreate))
	{
		if (std::filesystem::create_directories(toCreate))
		{
			LOG_DEBUG << "Directory '" << toCreate << "' successfully created.";
		}
		else
		{
			LOG_DEBUG << "Cannot create directory '" << toCreate << "'.";
			return false;
		}
	}

	return true;
}

bool StormPackager::PackagerHelper::copy(const std::filesystem::path &from, const std::filesystem::path &to)
{
	std::filesystem::copy(from, to, std::filesystem::copy_options::recursive & std::filesystem::copy_options::overwrite_existing);
	if (std::filesystem::exists(to))
	{
		LOG_DEBUG << "'" << from << "' successfully copied to '" << to << "'.";
		return true;
	}
	else
	{
		LOG_DEBUG << "Failed to copy '" << from << " from '" << to << "'.";
		return false;
	}
}

bool StormPackager::PackagerHelper::erase(const std::filesystem::path &toRemove)
{
	std::size_t removedCount = std::filesystem::remove_all(toRemove);
	if (std::filesystem::exists(toRemove))
	{
		LOG_DEBUG << "'" << toRemove << "' deletion failed.";
		return false;
	}
	else
	{
		if (removedCount == 1)
		{
			LOG_DEBUG << "'" << toRemove << "' successfully deleted.";
		}
		else if (removedCount > 1)
		{
			LOG_DEBUG << "'" << toRemove << "' successfully deleted. " << removedCount << " files/folders were deleted in the process.";
		}
		return true;
	}
}

