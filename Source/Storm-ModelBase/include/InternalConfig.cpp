#include "InternalConfig.h"

#include "GeneratedGitConfig.h"



Storm::GeneratedGitConfig::GeneratedGitConfig() = default;

Storm::InternalConfig::InternalConfig() :
	_generatedGitConfig{ std::make_unique<Storm::GeneratedGitConfig>() }
{}

Storm::InternalConfig::~InternalConfig() = default;
