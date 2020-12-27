#pragma once


namespace Storm
{
	enum class ThreadEnumeration;

	struct ScriptFileInitConfig
	{
	public:
		std::string _filePath;
	};

	struct ScriptFilePipeConfig
	{
	public:
		std::string _filePath;
		unsigned int _refreshRateInMillisec;
	};

	struct SceneScriptConfig
	{
	public:
		SceneScriptConfig();

	public:
		bool _enabled;

		Storm::ScriptFileInitConfig _initScriptFiles;
		Storm::ScriptFilePipeConfig _scriptFilePipe;
	};
}
