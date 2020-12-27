#pragma once


namespace Storm
{
	struct SceneSimulationConfig;
	struct SceneGraphicConfig;
	struct SceneFluidConfig;
	struct SceneRigidBodyConfig;
	struct SceneBlowerConfig;
	struct SceneConstraintConfig;
	struct SceneRecordConfig;
	struct SceneScriptConfig;

	struct SceneConfig
	{
	public:
		SceneConfig();
		~SceneConfig();

	public:
		std::unique_ptr<Storm::SceneSimulationConfig> _simulationConfig;
		std::unique_ptr<Storm::SceneGraphicConfig> _graphicConfig;
		std::unique_ptr<Storm::SceneFluidConfig> _fluidConfig;
		std::unique_ptr<Storm::SceneRecordConfig> _recordConfig;
		std::unique_ptr<Storm::SceneScriptConfig> _scriptConfig;
		std::vector<Storm::SceneRigidBodyConfig> _rigidBodiesConfig;
		std::vector<Storm::SceneBlowerConfig> _blowersConfig;
		std::vector<Storm::SceneConstraintConfig> _contraintsConfig;
	};
}
