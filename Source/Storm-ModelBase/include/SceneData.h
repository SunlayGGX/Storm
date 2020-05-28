#pragma once


namespace Storm
{
	struct RigidBodySceneData;
	struct GraphicData;

	struct SceneData
	{
	public:
		SceneData();

	public:
		Storm::Vector3 _gravity;

		std::unique_ptr<Storm::GraphicData> _graphicData;
		std::vector<Storm::RigidBodySceneData> _rigidBodiesData;
	};
}
