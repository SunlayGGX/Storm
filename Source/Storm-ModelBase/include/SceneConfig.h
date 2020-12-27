#pragma once

#include "SceneSimulationConfig.h"
#include "SceneGraphicConfig.h"
#include "SceneFluidConfig.h"
#include "SceneRecordConfig.h"
#include "SceneScriptConfig.h"


namespace Storm
{
	struct SceneRigidBodyConfig;
	struct SceneBlowerConfig;
	struct SceneConstraintConfig;

	struct SceneConfig
	{
	public:
		~SceneConfig();

	public:
		Storm::SceneSimulationConfig _simulationConfig;
		Storm::SceneGraphicConfig _graphicConfig;
		Storm::SceneFluidConfig _fluidConfig;
		Storm::SceneRecordConfig _recordConfig;
		Storm::SceneScriptConfig _scriptConfig;

		std::vector<Storm::SceneRigidBodyConfig> _rigidBodiesConfig;
		std::vector<Storm::SceneBlowerConfig> _blowersConfig;
		std::vector<Storm::SceneConstraintConfig> _contraintsConfig;
	};
}
