#pragma once

#include "SceneSimulationConfig.h"
#include "ScenePhysicsConfig.h"
#include "SceneGraphicConfig.h"
#include "SceneFluidConfig.h"
#include "SceneRecordConfig.h"
#include "SceneScriptConfig.h"


namespace Storm
{
	struct SceneRigidBodyConfig;
	struct SceneBlowerConfig;
	struct SceneConstraintConfig;
	struct SceneCageConfig;

	struct SceneConfig
	{
	public:
		SceneConfig();
		~SceneConfig();

	public:
		Storm::SceneSimulationConfig _simulationConfig;
		Storm::SceneGraphicConfig _graphicConfig;
		Storm::ScenePhysicsConfig _physicsConfig;
		Storm::SceneFluidConfig _fluidConfig;
		Storm::SceneRecordConfig _recordConfig;
		Storm::SceneScriptConfig _scriptConfig;

		std::unique_ptr<Storm::SceneCageConfig> _optionalCageConfig;

		std::vector<Storm::SceneRigidBodyConfig> _rigidBodiesConfig;
		std::vector<Storm::SceneBlowerConfig> _blowersConfig;
		std::vector<Storm::SceneConstraintConfig> _contraintsConfig;
	};
}
