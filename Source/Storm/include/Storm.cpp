#include "Application.h"

#include <iostream>


namespace
{
	int processEarlyExitAnswer(const Storm::EarlyExitAnswer &earlyExitAnswer)
	{
		for (const auto &unhandledErrorMsg : earlyExitAnswer._unconsumedErrorMsgs)
		{
			std::cerr << unhandledErrorMsg << std::endl;
		}

		return earlyExitAnswer._exitCode;
	}
}

int main(const int argc, const char* argv[]) try
{
	return static_cast<int>(Storm::Application{ argc, argv }.run());
}
catch (const Storm::Exception &ex)
{
	return processEarlyExitAnswer(
		Storm::Application::ensureCleanStateAfterException(
			"Fatal error (Storm::Exception received) : " + std::string{ ex.what() } + ".\n" + ex.stackTrace(),
			true
		)
	);
}
catch (const std::exception &ex)
{
	return processEarlyExitAnswer(Storm::Application::ensureCleanStateAfterException("Fatal error (std::exception received) : " + std::string{ ex.what() }, true));
}
catch (...)
{
	return processEarlyExitAnswer(Storm::Application::ensureCleanStateAfterException("Fatal error (unknown exception received)", false));
}
