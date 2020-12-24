#pragma once


namespace Storm
{
	class ScriptObject;

	class CommandParser
	{
	public:
		static std::vector<Storm::ScriptObject> parse(std::string &&totalScriptCommand);
	};
}
