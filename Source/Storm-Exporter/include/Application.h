#pragma once


namespace Storm
{
	enum class ExitCode : int;
}


namespace StormExporter
{
	class Application
	{
	public:
		Application(int argc, const char*const argv[]);
		~Application();

	public:
		Storm::ExitCode run();

		static void staticForceShutdown();
	};
}
