#pragma once


namespace Storm
{
	struct SceneConfig;
	struct ConfigReadParam;
	class MacroConfigHolder;
	class GeneralConfigHolder;

	class SceneConfigHolder
	{
	public:
		void read(const std::string &sceneConfigFilePathStr, const Storm::MacroConfigHolder &macroConfig, const Storm::GeneralConfigHolder &generalConfigHolder, const Storm::ConfigReadParam &param);

	public:
		const Storm::SceneConfig& getConfig() const;
		Storm::SceneConfig& getConfig();

	private:
		std::unique_ptr<Storm::SceneConfig> _sceneConfig;
	};
}
