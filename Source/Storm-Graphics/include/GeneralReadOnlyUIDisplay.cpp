#include "GeneralReadOnlyUIDisplay.h"

#include "IConfigManager.h"
#include "SingletonHolder.h"

#include "SceneSimulationConfig.h"

#include "UIField.h"
#include "UIFieldContainer.h"

#include "GraphicHelpers.h"


namespace
{
	struct CustomFieldParser
	{
	public:
		static std::wstring parseToWString(const Storm::Vector3 &val)
		{
			std::wstring result;

			std::wstring xWStr = std::to_wstring(val.x());
			Storm::GraphicHelpers::removeUselessZeros(xWStr);

			std::wstring yWStr = std::to_wstring(val.y());
			Storm::GraphicHelpers::removeUselessZeros(yWStr);

			std::wstring zWStr = std::to_wstring(val.z());
			Storm::GraphicHelpers::removeUselessZeros(zWStr);

			result.reserve(4 + xWStr.size() + yWStr.size() + zWStr.size());

			result += xWStr;
			result += L", ";
			result += yWStr;
			result += L", ";
			result += zWStr;

			return result;
		}
	};
}


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
