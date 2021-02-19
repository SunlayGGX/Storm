#include "GeneralReadOnlyUIDisplay.h"

#include "IConfigManager.h"
#include "SingletonHolder.h"

#include "SceneSimulationConfig.h"

#include "UIField.h"
#include "UIFieldContainer.h"


Storm::GeneralReadOnlyUIDisplay::GeneralReadOnlyUIDisplay() :
	_readOnlyFields{ std::make_unique<Storm::UIFieldContainer>() }
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

	const Storm::SceneSimulationConfig &simulConfig = configMgr.getSceneSimulationConfig();

	(*_readOnlyFields)
		.bindField("Gravity", simulConfig._gravity)
		;
}

Storm::GeneralReadOnlyUIDisplay::~GeneralReadOnlyUIDisplay() = default;
