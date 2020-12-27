#pragma once


namespace Storm
{
	struct StormProcessStartup
	{
	public:
		using OutPipeCallback = std::function<bool(const std::string &)>;

	public:
		std::string _exePath;
		std::string _workingDirectoryPath;

		std::string _commandLine;

		Storm::StormProcessStartup::OutPipeCallback _pipeCallback;
		bool _bindIO;

		bool _shareLife;

		bool _isCmd;
	};
}
