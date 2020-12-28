#include "GeneratedWrapper.h"

#include "GeneratedGitConfig.h"

#include "GitBranch.generated.h"


void Storm::initGitGeneratedConfig(Storm::GeneratedGitConfig &config)
{
	config._gitBranchStr = STORM_CURRENT_GIT_BRANCH;
	config._gitBranchWStr = STORM_TEXT(STORM_CURRENT_GIT_BRANCH);

	LOG_DEBUG << "Git auto generated settings were initialized : We run the application built from '" << config._gitBranchStr << "'";
}
