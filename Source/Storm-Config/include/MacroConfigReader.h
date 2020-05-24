#pragma once


namespace Storm
{
	class MacroConfigFileReader
	{
	private:
		using MacroKey = std::string;
		using MacroValue = std::string;
		
	public:
		MacroConfigFileReader();

	public:
		void initialize();

	public:
		bool read(const std::string &macroConfigFilePathStr);

	public:
		const std::string*const queryMacroValue(const std::string &key) const;

	public:
		void operator()(std::string &inOutStr) const;
		std::string operator()(const std::string &inStr) const;

	private:
		std::map<MacroKey, MacroValue> _macros;
		mutable bool _lastHasResolved;
	};
}
