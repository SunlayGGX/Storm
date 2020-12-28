#include "GeneralConfig.h"

#include "GeneralGraphicConfig.h"
#include "GeneralSimulationConfig.h"
#include "GeneralWebConfig.h"
#include "GeneralDebugConfig.h"
#include "GeneralApplicationConfig.h"

#include "PreferredBrowser.h"
#include "VectoredExceptionDisplayMode.h"


Storm::GeneralGraphicConfig::GeneralGraphicConfig() :
	_wantedApplicationHeight{ 800 },
	_wantedApplicationWidth{ 1200 },
	_fontSize{ 17.f },
	_wantedApplicationXPos{ std::numeric_limits<int>::max() },
	_wantedApplicationYPos{ std::numeric_limits<int>::max() },
	_fixNearFarPlanesWhenTranslating{ true },
	_selectedParticleShouldBeTopMost{ false },
	_selectedParticleForceShouldBeTopMost{ true }
{}

Storm::GeneralSimulationConfig::GeneralSimulationConfig() :
	_allowNoFluid{ false }
{}

Storm::GeneralWebConfig::GeneralWebConfig() :
	_urlOpenIncognito{ false },
	_preferredBrowser{ Storm::PreferredBrowser::None }
{}

Storm::GeneralDebugConfig::GeneralDebugConfig() :
	_logLevel{ Storm::LogLevel::Debug },
	_overrideLogs{ true },
	_removeLogsOlderThanDays{ -1 },
	_shouldLogFPSWatching{ false },
	_shouldLogGraphicDeviceMessage{ false },
	_shouldLogPhysics{ false },
	_profileSimulationSpeed{ false },
	_displayVectoredExceptions{ Storm::VectoredExceptionDisplayMode::DisplayFatal }
{}

Storm::GeneralApplicationConfig::GeneralApplicationConfig() :
	_showBranchInTitle{ false }
{

}
