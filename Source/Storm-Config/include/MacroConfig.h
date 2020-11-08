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

	private:
		const std::pair<const MacroKey, MacroValue>& registerMacroInternal(const std::string &key, std::string value);

	public:
		const std::string*const queryMacroValue(const std::string &key) const;
		void registerMacro(const std::string &key, std::string value, bool shouldLog = true);
		
		void resolveInternalMacro();

	private:
		std::string produceMacroLog(const std::vector<MacroKey> &selectedMacros) const;
		std::string produceAllMacroLog() const;

	public:
		void operator()(std::string &inOutStr) const;
		std::string operator()(const std::string &inStr) const;

	private:
		std::map<MacroKey, MacroValue> _macros;
		mutable bool _lastHasResolved;
	};
}
