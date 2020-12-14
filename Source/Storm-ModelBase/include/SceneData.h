#pragma once


namespace Storm
{
	struct GeneralSimulationData;
	struct GraphicData;
	struct FluidData;
	struct RigidBodySceneData;
	struct BlowerData;
	struct ConstraintData;
	struct RecordConfigData;
	struct ScriptData;

	struct SceneData
	{
	public:
		SceneData();
		~SceneData();

	public:
		std::unique_ptr<Storm::GeneralSimulationData> _generalSimulationData;
		std::unique_ptr<Storm::GraphicData> _graphicData;
		std::unique_ptr<Storm::FluidData> _fluidData;
		std::unique_ptr<Storm::RecordConfigData> _recordConfigData;
		std::unique_ptr<Storm::ScriptData> _scriptConfigData;
		std::vector<Storm::RigidBodySceneData> _rigidBodiesData;
		std::vector<Storm::BlowerData> _blowersData;
		std::vector<Storm::ConstraintData> _contraintsData;
	};
}
