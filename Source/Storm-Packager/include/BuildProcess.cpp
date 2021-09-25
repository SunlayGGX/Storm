#include "BuildProcess.h"

#include "ExecHelper.h"


bool StormPackager::BuildProcess::execute(const std::string &devenvPath, const std::string &sln)
{
	std::string fullCommand;
	fullCommand.reserve(devenvPath.size() + sln.size() + 16);

	fullCommand += '"';
	fullCommand += devenvPath;
	fullCommand += "\" \"";

	fullCommand += sln; 
	fullCommand += "\" /Build ReleaseNoPackager|x64";

	LOG_DEBUG << "Building '" << sln << "'. This will take some time but please, do not stop the application.";

	const auto buildOutput = StormPackager::ExecHelper::execute(fullCommand);
	if (buildOutput._success && buildOutput._commandExitCode == 0)
	{
		LOG_DEBUG << "'" << sln << "' was built successfully!";
		return true;
	}
	else
	{
		LOG_ERROR <<
			"'" << sln << "' build has failed with exit code " << buildOutput._commandExitCode << ".\n"
			"Command line was: '" << fullCommand << "'.\n\n"
			"Error: '" << buildOutput._error << "'.\n\n"
			"Output: '" << buildOutput._output << "'."
			;
		return false;
	}
}
