#pragma once


namespace Storm
{
	struct RigidBodySceneData;

	struct SceneData
	{
	public:
		SceneData();

	public:
		Storm::Vector3 _gravity;

		std::vector<Storm::RigidBodySceneData> _rigidBodiesData;
	};
}
