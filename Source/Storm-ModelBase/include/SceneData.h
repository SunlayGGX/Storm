#pragma once


namespace Storm
{
	struct GeneralSimulationData;
	struct GraphicData;
	struct RigidBodySceneData;

	struct SceneData
	{
	public:
		SceneData();

	public:
		std::unique_ptr<Storm::GeneralSimulationData> _generalSimulationData;
		std::unique_ptr<Storm::GraphicData> _graphicData;
		std::vector<Storm::RigidBodySceneData> _rigidBodiesData;
	};
}
