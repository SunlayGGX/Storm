#pragma once


namespace Storm
{
	class MacroConfig;
	struct GeneralConfig;

	class GeneralConfigHolder
	{
	public:
		GeneralConfigHolder();

	public:
		bool read(const std::string &generalConfigFilePathStr);

		void applyMacros(const Storm::MacroConfig &macroConf);

	public:
		const Storm::GeneralConfig& getConfig() const;
		Storm::GeneralConfig& getConfig();

	private:
		std::unique_ptr<Storm::GeneralConfig> _generalConfig;
	};
}
