#pragma once


namespace Storm
{
	enum class ThreadEnumeration;

	struct ScriptFilePipeData
	{
	public:
		Storm::ThreadEnumeration _threadEnum;
		std::string _filePath;
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
		std::vector<Storm::ScriptFilePipeData> _scriptFilePipes;
	};
}
