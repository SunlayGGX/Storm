#pragma once


namespace Storm
{
	struct SceneData;
	class SceneConfig
	{
	public:
		bool read(const std::string &sceneConfigFilePathStr);

	public:
		const Storm::SceneData& getSceneData() const;

	private:
		std::unique_ptr<Storm::SceneData> _sceneData;
	};
}
