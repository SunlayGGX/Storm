#include "CleanTask.h"

#include "ConfigManager.h"

#include "PackagerHelper.h"



std::string_view StormPackager::CleanTask::getName() const
{
	return "Cleaner";
}

std::string StormPackager::CleanTask::prepare()
{
	std::string result;

	const StormPackager::ConfigManager &configMgr = StormPackager::ConfigManager::instance();
	const std::string &destination = configMgr.getDestinationPackageFolderPath();
	const std::string &tempPath = configMgr.getTmpPath();

	if (!StormPackager::PackagerHelper::erase(destination))
	{
		result += "Destination folder " + destination + " deletion failed!";
	}
	if (!StormPackager::PackagerHelper::createDirectory(destination))
	{
		result += "Error happened when creating " + destination + " folder!";
	}

	if (!StormPackager::PackagerHelper::erase(tempPath))
	{
		result += "Temporary folder " + tempPath + " deletion failed!";
	}
	if (!StormPackager::PackagerHelper::createDirectory(tempPath))
	{
		result += "Error happened when creating " + tempPath + " folder!";
	}

	return result;
}

std::string StormPackager::CleanTask::execute()
{
	return std::string{};
}

std::string StormPackager::CleanTask::cleanUp()
{
	return std::string{};
}
