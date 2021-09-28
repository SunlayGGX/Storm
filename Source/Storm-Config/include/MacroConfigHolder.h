#pragma once


namespace Storm
{
	class MacroConfigHolder
	{
	private:
		using MacroKey = std::string;
		using MacroValue = std::string;
		
	public:
		MacroConfigHolder();

	public:
		void initialize();

	public:
		bool read(const std::string &macroConfigFilePathStr);

	private:
		const std::pair<const MacroKey, MacroValue>& registerMacroInternal(const std::string_view key, std::string value);

	public:
		const std::string*const queryMacroValue(const std::string &key) const;
		void registerMacro(const std::string &key, std::string value, bool shouldLog = true);
		
		void resolveInternalMacro();

	private:
		std::string produceMacroLog(const std::vector<MacroKey> &selectedMacros) const;
		std::string produceAllMacroLog() const;

	public:
		bool hasKnownMacro(const std::string &inOutStr, std::size_t &outPosFound) const;

	public:
		std::string makeMacroKey(const std::string_view value) const;

	public:
		void operator()(std::string &inOutStr) const;
		std::string operator()(const std::string &inStr) const;

		void operator()(std::string &inOutStr, const std::size_t beginSearchPos) const;

	private:
		std::map<MacroKey, MacroValue> _macros;
		mutable bool _lastHasResolved;
	};
}
