#include "ZipperTask.h"

#include "ConfigManager.h"


namespace
{
	std::string buildZipCommand(const std::string &src, const std::string &dest)
	{
		std::string result;

		result.reserve(src.size() + dest.size() + 160);

		result += "powershell.exe -nologo -noprofile -command \"& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::CreateFromDirectory('";
		result += src;
		result += "', '";
		result += dest;
		result += "'); }\"";

		return result;
	}
}



std::string_view StormPackager::ZipperTask::getName() const
{
	return "Zipper";
}

std::string StormPackager::ZipperTask::prepare()
{
	return std::string{};
}

std::string StormPackager::ZipperTask::execute()
{
	std::string result;

	const StormPackager::ConfigManager &configMgr = StormPackager::ConfigManager::instance();
	const std::string &srcToZip = configMgr.getTmpPath();
	const std::filesystem::path destZip = std::filesystem::path{ configMgr.getDestinationPackageFolderPath() } / std::string{ configMgr.getDestinationPackageName() + ".zip"};
	const std::string dest = destZip.string();

	const std::string zipCommand = buildZipCommand(srcToZip, dest);

	const int exitCode = ::system(zipCommand.c_str());
	if (exitCode == 0)
	{
		if (std::filesystem::exists(destZip))
		{
			LOG_DEBUG << "Zip process successfully finished. Zip ";
		}
		else
		{
			result = "zip task ended in failure because the packaged zip doesn't exist! (" + dest + ")";
		}
	}
	else
	{
		result = "zip task ended in failure with exit code " + std::to_string(exitCode);
	}

	return result;
}

std::string StormPackager::ZipperTask::cleanUp()
{
	return std::string{};
}
