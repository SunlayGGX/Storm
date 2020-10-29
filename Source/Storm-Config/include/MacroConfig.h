#pragma once


namespace Storm
{
	class MacroConfig
	{
	private:
		using MacroKey = std::string;
		using MacroValue = std::string;
		
	public:
		MacroConfig();

	public:
		void initialize();

	public:
		bool read(const std::string &macroConfigFilePathStr);

	public:
		const std::string*const queryMacroValue(const std::string &key) const;
		
		void resolveInternalMacro();

	public:
		void operator()(std::string &inOutStr) const;
		std::string operator()(const std::string &inStr) const;

	private:
		std::map<MacroKey, MacroValue> _macros;
		mutable bool _lastHasResolved;
	};
}
