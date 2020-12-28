#pragma once

#include "GeneralGraphicConfig.h"
#include "GeneralSimulationConfig.h"
#include "GeneralWebConfig.h"
#include "GeneralDebugConfig.h"
#include "GeneralApplicationConfig.h"


namespace Storm
{
	struct GeneralConfig
	{
	public:
		Storm::GeneralGraphicConfig _generalGraphicConfig;
		Storm::GeneralSimulationConfig _generalSimulationConfig;
		Storm::GeneralWebConfig _generalWebConfig;
		Storm::GeneralDebugConfig _generalDebugConfig;
		Storm::GeneralApplicationConfig _generalApplicationConfig;
	};
}
