#pragma once


namespace Storm
{
	struct SceneConfig;
	class MacroConfig;
	class GeneralConfig;

	class SceneConfigHolder
	{
	public:
		void read(const std::string &sceneConfigFilePathStr, const Storm::MacroConfig &macroConfig, const Storm::GeneralConfig &generalConfig);

	public:
		const Storm::SceneConfig& getSceneConfig() const;

	private:
		std::unique_ptr<Storm::SceneConfig> _sceneConfig;
	};
}
