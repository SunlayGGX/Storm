#include "InternalConfigHolder.h"

#include "InternalConfig.h"

#include "GeneratedWrapper.h"


Storm::InternalConfigHolder::InternalConfigHolder() :
	_internalConfig{ std::make_unique<Storm::InternalConfig>() }
{

}

Storm::InternalConfigHolder::~InternalConfigHolder() = default;

void Storm::InternalConfigHolder::init()
{
	Storm::initGitGeneratedConfig(*_internalConfig->_generatedGitConfig);
}

const Storm::InternalConfig& Storm::InternalConfigHolder::getInternalConfig() const
{
	return *_internalConfig;
}
