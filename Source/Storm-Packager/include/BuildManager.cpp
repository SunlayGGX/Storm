#include "BuildManager.h"

#include "ConfigManager.h"

#include "BuildProcess.h"

#include "ExecHelper.h"


StormPackager::BuildManager::BuildManager() = default;
StormPackager::BuildManager::~BuildManager() = default;

bool StormPackager::BuildManager::initialize_Implementation()
{
	const StormPackager::ConfigManager &configMgr = StormPackager::ConfigManager::instance();

	_branchToBuild = configMgr.getBuildBranch();
	if (_branchToBuild.empty())
	{
		return false;
	}

	auto gitCommandExecResult = StormPackager::ExecHelper::getCurrentBranchName();
	if (!gitCommandExecResult._error.empty())
	{
		LOG_ERROR <<
			"Cannot retrieve the current git branch. We won't build since we cannot come back! Error was :\n" <<
			gitCommandExecResult._error
			;
		return false;
	}
	else if (!gitCommandExecResult._success || gitCommandExecResult._output.empty())
	{
		LOG_ERROR <<
			"Cannot retrieve the current git branch. We won't build since we cannot come back!\n"
			"Error was unknown (we couldn't get the branch name, that is all)."
			;
		return false;
	}
	else
	{
		_oldBranch = std::move(gitCommandExecResult._output);
	}

	const std::filesystem::path stormRootPath = configMgr.getStormRootPath();
	const std::filesystem::path vswhereExePath = stormRootPath / "Build" / "Tools" / "vswhere" / "vswhere.exe";

	auto vswhereCommandExecResult = StormPackager::ExecHelper::execute(vswhereExePath.string() + " -latest -property productPath");
	if (!vswhereCommandExecResult._error.empty())
	{
		LOG_ERROR << 
			"Couldn't locate Visual Studio devenv executable. Build (along with packaging process) will be aborted.\n"
			"Error: " << vswhereCommandExecResult._error
			;
		return false;
	}
	else if (vswhereCommandExecResult._output.empty() || !vswhereCommandExecResult._success)
	{
		LOG_ERROR << "Couldn't locate Visual Studio devenv executable. Build (along with packaging process) will be aborted. Error is unknown.";
		return false;
	}

	_visualStudioLocation = std::move(vswhereCommandExecResult._output);
	const std::filesystem::path devenvPath = _visualStudioLocation;
	if (!std::filesystem::exists(devenvPath) || devenvPath.extension() != ".exe" && devenvPath.stem() != "devenv")
	{
		LOG_ERROR << "Something is weird with the devenv location returned from vswhere : " << devenvPath << ". Aborting...";
		return false;
	}

	_buildProcess = std::make_unique<StormPackager::BuildProcess>();
	return true;
}

void StormPackager::BuildManager::cleanUp_Implementation()
{
	if (_oldBranch != _branchToBuild)
	{
		const auto revertCheckoutResult = StormPackager::ExecHelper::checkout(_oldBranch);
		if (!revertCheckoutResult._success && !revertCheckoutResult._error.empty())
		{
			LOG_ERROR <<
				"Something went wrong with checkout to old branch " << _oldBranch << ".\n"
				"Error was " << revertCheckoutResult._error;
		}
		else
		{
			LOG_DEBUG <<
				"Successfully reverted branch to " << _oldBranch << ".\n" <<
				revertCheckoutResult._error << "\n\n" <<
				revertCheckoutResult._output << "\n\n"
				;
		}
	}

	_buildProcess.reset();
}

bool StormPackager::BuildManager::run()
{
	if (_buildProcess == nullptr)
	{
		return true;
	}

	if (_branchToBuild != _oldBranch)
	{
		const auto gitCommandExecResult = StormPackager::ExecHelper::checkout(_branchToBuild);
		if (!gitCommandExecResult._success && !gitCommandExecResult._error.empty())
		{
			LOG_ERROR <<
				"Something went wrong with checking out to branch " << _branchToBuild << ".\n"
				"Error was " << gitCommandExecResult._error << ".\n\n"
				"Output was " << gitCommandExecResult._output << "."
				;
		}
		else
		{
			LOG_DEBUG <<
				"Successfully checkout to branch " << _branchToBuild << ".\n" <<
				gitCommandExecResult._error << "\n\n" <<
				gitCommandExecResult._output << "\n\n"
				;
		}
	}
	else
	{
		LOG_DEBUG << "Checkout to branch '" << _branchToBuild << "' skipped because we are already on this branch.";
	}

	const StormPackager::ConfigManager &configMgr = StormPackager::ConfigManager::instance();
	const std::filesystem::path slnToBuild = std::filesystem::path{ configMgr.getStormRootPath() } / "Storm.sln";

	return
		std::filesystem::exists(slnToBuild) &&
		_buildProcess->execute(_visualStudioLocation, slnToBuild.string());
}

