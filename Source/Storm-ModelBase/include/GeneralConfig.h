#pragma once

#include "GeneralGraphicConfig.h"
#include "GeneralSimulationConfig.h"
#include "GeneralWebConfig.h"
#include "GeneralDebugConfig.h"
#include "GeneralApplicationConfig.h"
#include "GeneralNetworkConfig.h"
#include "GeneralSafetyConfig.h"
#include "GeneralArchiveConfig.h"


namespace Storm
{
	struct GeneralConfig
	{
	public:
		Storm::GeneralGraphicConfig _generalGraphicConfig;
		Storm::GeneralSimulationConfig _generalSimulationConfig;
		Storm::GeneralNetworkConfig _generalNetworkConfig;
		Storm::GeneralWebConfig _generalWebConfig;
		Storm::GeneralDebugConfig _generalDebugConfig;
		Storm::GeneralApplicationConfig _generalApplicationConfig;
		Storm::GeneralSafetyConfig _generalSafetyConfig;
		Storm::GeneralArchiveConfig _generalArchiveConfig;
	};
}
