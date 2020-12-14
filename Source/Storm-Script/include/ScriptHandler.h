#pragma once


namespace Storm
{
	class ScriptHandler
	{
	public:
		virtual ~ScriptHandler() = default;

	public:
		virtual void execute(const std::string &script) = 0;
	};
}
