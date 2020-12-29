#include "InternalConfig.h"

#include "GeneratedGitConfig.h"
#include "InternalReferenceConfig.h"


Storm::InternalReferenceConfig::InternalReferenceConfig() :
	_type{ Storm::PaperType::Unknown },
	_id{ 0 }
{

}

Storm::GeneratedGitConfig::GeneratedGitConfig() = default;

Storm::InternalConfig::InternalConfig() :
	_generatedGitConfig{ std::make_unique<Storm::GeneratedGitConfig>() }
{}

Storm::InternalConfig::~InternalConfig() = default;
