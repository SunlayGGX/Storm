#pragma once


namespace Storm
{
	struct SceneData;
	class MacroConfig;
	class GeneralConfig;

	class SceneConfig
	{
	public:
		void read(const std::string &sceneConfigFilePathStr, const Storm::MacroConfig &macroConfig, const Storm::GeneralConfig &generalConfig);

	public:
		const Storm::SceneData& getSceneData() const;

	private:
		std::unique_ptr<Storm::SceneData> _sceneData;
	};
}
