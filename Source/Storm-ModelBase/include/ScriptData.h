#pragma once


namespace Storm
{
	enum class ThreadEnumeration;

	struct ScriptFilePipeData
	{
	public:
		std::string _filePath;
		unsigned int _refreshRateInMillisec;
	};

	struct ScriptFileInitData
	{
	public:
		std::string _filePath;
	};

	struct ScriptData
	{
	public:
		ScriptData();

	public:
		bool _enabled;

		Storm::ScriptFileInitData _initScriptFiles;
		Storm::ScriptFilePipeData _scriptFilePipe;
	};
}
