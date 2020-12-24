#pragma once


namespace Storm
{
	class CommandItem;

	class ScriptObject
	{
	public:
		ScriptObject(std::string &&script);
		~ScriptObject();

	public:
		const std::string& getScript() const noexcept;

	private:
		std::string _script;
	};
}
