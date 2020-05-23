#pragma once

#include "ExitCode.h"


namespace Storm
{
	class Application
	{
	public:
		Application(int argc, const char* argv[]);
		~Application();

		Storm::ExitCode run();
	};
}
