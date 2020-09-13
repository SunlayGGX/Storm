#pragma once


namespace Storm
{
	enum class ExitCode;
}

namespace StormPackager
{
	class Application
	{
	public:
		Application(int argc, const char*const argv[]);
		~Application();

	public:
		Storm::ExitCode run();
	};
}
