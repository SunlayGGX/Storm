#include "PackagerManager.h"

#include "ITaskLogic.h"
#include "CleanTask.h"
#include "CopierTask.h"
#include "ZipperTask.h"


StormPackager::PackagerManager::PackagerManager() :
	_prepared{ false }
{
	_taskList.reserve(3);
	_taskList.emplace_back(std::make_unique<StormPackager::CleanTask>());
	_taskList.emplace_back(std::make_unique<StormPackager::CopierTask>());
	_taskList.emplace_back(std::make_unique<StormPackager::ZipperTask>());
}

StormPackager::PackagerManager::~PackagerManager() = default;

void StormPackager::PackagerManager::initialize_Implementation()
{
	LOG_COMMENT << "Preparing packaging";

	std::string errorMsg;
	for (auto &packagerLogic : _taskList)
	{
		const std::string_view packagerName = packagerLogic->getName();
		LOG_COMMENT << "Executing " << packagerName << " preparation.";

		errorMsg = packagerLogic->prepare();
		if (!errorMsg.empty())
		{
			LOG_ERROR << "Packager " << packagerName << " logic failed to prepare. We will abort packaging. Error was " << errorMsg;
			return;
		}
	}

	LOG_COMMENT << "Preparing done successfully";

	_prepared = true;
}

void StormPackager::PackagerManager::cleanUp_Implementation()
{
	LOG_COMMENT << "Starting cleanUp";

	using ErrorInfo = std::pair<std::string, std::string>;

	std::vector<ErrorInfo> errorMsgList;
	errorMsgList.reserve(_taskList.size());

	for (auto &packagerLogic : _taskList)
	{
		const std::string_view packagerName = packagerLogic->getName();
		LOG_COMMENT << "Executing " << packagerName << " cleanUp.";

		std::string errorMsg = packagerLogic->cleanUp();
		if (!errorMsg.empty())
		{
			errorMsgList.emplace_back(packagerName, std::move(errorMsg));
		}
	}

	if (!errorMsgList.empty())
	{
		for (const ErrorInfo &errorInfo : errorMsgList)
		{
			LOG_ERROR << "Packager " << errorInfo.first << " logic failed to cleanup. Error was " << errorInfo.second;
		}
	}
}

bool StormPackager::PackagerManager::run()
{
	if (!_prepared)
	{
		return false;
	}

	LOG_COMMENT << "Starting packaging run";

	std::string errorMsg;
	for (auto &packagerLogic : _taskList)
	{
		const std::string_view packagerName = packagerLogic->getName();
		LOG_COMMENT << "Running " << packagerName << ".";

		errorMsg = packagerLogic->execute();
		if (!errorMsg.empty())
		{
			LOG_ERROR << "Packager " << packagerName << " logic failed to execute. We will abort packaging. Error was " << errorMsg;
			return false;
		}
	}

	LOG_COMMENT << "Packaging run successfully";

	return true;
}
