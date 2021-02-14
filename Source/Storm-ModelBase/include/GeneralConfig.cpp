#include "GeneralConfig.h"

#include "GeneralGraphicConfig.h"
#include "GeneralSimulationConfig.h"
#include "GeneralWebConfig.h"
#include "GeneralDebugConfig.h"
#include "GeneralApplicationConfig.h"

#include "PreferredBrowser.h"
#include "VectoredExceptionDisplayMode.h"

#include "SocketSetting.h"


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
	_displayVectoredExceptions{ Storm::VectoredExceptionDisplayMode::DisplayFatal },
	_physXPvdDebugSocketSettings{ std::make_unique<Storm::SocketSetting>("127.0.0.1", 5425) },
	_pvdConnectTimeoutMillisec{ 33 },
	_pvdTransmitConstraints{ true },
	_pvdTransmitContacts{ true },
	_pvdTransmitSceneQueries{ false }
{}

Storm::GeneralApplicationConfig::GeneralApplicationConfig() :
	_showBranchInTitle{ false }
{

}

Storm::GeneralDebugConfig::~GeneralDebugConfig() = default;
