#include "SceneConfig.h"



bool Storm::SceneConfig::read(const std::string &sceneConfigFilePathStr)
{
	const std::filesystem::path sceneConfigFilePath{ sceneConfigFilePathStr };
	if (std::filesystem::is_regular_file(sceneConfigFilePath))
	{
		if (sceneConfigFilePath.extension() == ".xml")
		{
			// TODO
			throw std::runtime_error{ "non implemented yet" };
			return true;
		}
	}

	return false;
}