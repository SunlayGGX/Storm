#pragma once


namespace Storm
{
	struct SceneData;
	class MacroConfig;

	class SceneConfig
	{
	public:
		bool read(const std::string &sceneConfigFilePathStr, const Storm::MacroConfig &macroConfig);

	public:
		const Storm::SceneData& getSceneData() const;

	private:
		std::unique_ptr<Storm::SceneData> _sceneData;
	};
}
