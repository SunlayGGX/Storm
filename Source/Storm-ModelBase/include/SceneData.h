#pragma once


namespace Storm
{
	struct GeneralSimulationData;
	struct GraphicData;
	struct FluidData;
	struct RigidBodySceneData;
	struct BlowerData;

	struct SceneData
	{
	public:
		SceneData();
		~SceneData();

	public:
		std::unique_ptr<Storm::GeneralSimulationData> _generalSimulationData;
		std::unique_ptr<Storm::GraphicData> _graphicData;
		std::unique_ptr<Storm::FluidData> _fluidData;
		std::vector<Storm::RigidBodySceneData> _rigidBodiesData;
		std::vector<Storm::BlowerData> _blowersData;
	};
}
