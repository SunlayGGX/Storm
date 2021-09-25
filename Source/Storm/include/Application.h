#pragma once

#include "ExitCode.h"
#include "EarlyExitAnswer.h"


namespace Storm
{
	class Application
	{
	public:
		Application(const int argc, const char* argv[]);
		~Application();

		Storm::ExitCode run();

		static Storm::EarlyExitAnswer ensureCleanStateAfterException(const std::string &errorMsg, bool wasStdException);
	};
}
