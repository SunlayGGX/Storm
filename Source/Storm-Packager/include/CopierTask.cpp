#include "CopierTask.h"

#include "ConfigManager.h"

#include "PackagerHelper.h"


std::string_view StormPackager::CopierTask::getName() const
{
	return "Copier";
}

std::string StormPackager::CopierTask::prepare()
{
	std::string result;

	const StormPackager::ConfigManager &configMgr = StormPackager::ConfigManager::instance();
	const std::filesystem::path &rootPath = configMgr.getStormRootPath();
	const std::filesystem::path &tempFolderPath = configMgr.getTmpPath();
	const std::vector<std::string> &toCopyPaths = configMgr.getToCopyPath();

	const std::size_t toCopyCount = toCopyPaths.size();

	LOG_DEBUG << "Preparing copy " << toCopyCount << " items and their subitems.";

	std::size_t fileCountNotExist = 0;

	result.reserve(toCopyCount * 64);
	for (const std::string &toCopyPathStr : toCopyPaths)
	{
		std::filesystem::path relativeToCopyPath{ toCopyPathStr };
		std::filesystem::path absoluteToCopyPath{ rootPath / relativeToCopyPath };
		if (std::filesystem::exists(absoluteToCopyPath))
		{
			if (std::filesystem::is_directory(absoluteToCopyPath))
			{
				_foldersToCreate.emplace(tempFolderPath / toCopyPathStr);
			}
			else
			{
				_foldersToCreate.emplace(tempFolderPath / relativeToCopyPath.parent_path());
			}

			_toCopy.emplace_back(std::move(absoluteToCopyPath), tempFolderPath / relativeToCopyPath);
		}
		else
		{
			result += "Error: '";
			result += toCopyPathStr;
			result += "' doesn't exist!\n";

			++fileCountNotExist;
		}
	}

	if (!result.empty())
	{
		result += "\nTotal missing file : " + std::to_string(fileCountNotExist);
	}

	return result;
}

std::string StormPackager::CopierTask::execute()
{
	std::string result;

	std::size_t directoryCreationFailureCount = 0;

	for (const std::filesystem::path &folderToCreate : _foldersToCreate)
	{
		if (!StormPackager::PackagerHelper::createDirectory(folderToCreate))
		{
			++directoryCreationFailureCount;
		}
	}

	if (directoryCreationFailureCount > 0)
	{
		result += "We failed to create " + std::to_string(directoryCreationFailureCount) + " directories...";
	}
	else
	{
		std::size_t copyFailureCount = 0;

		for (const auto &toCopyPair : _toCopy)
		{
			if (!StormPackager::PackagerHelper::copy(toCopyPair.first, toCopyPair.second))
			{
				++copyFailureCount;
			}
		}

		if (copyFailureCount > 0)
		{
			result += "We failed to copy " + std::to_string(copyFailureCount) + " items and their subitems...";
		}
	}

	return result;
}

std::string StormPackager::CopierTask::cleanUp()
{
	return std::string{};
}
