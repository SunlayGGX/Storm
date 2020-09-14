#include "PackagerManager.h"

#include "IPackagingLogic.h"
#include "ValidaterPackager.h"
#include "CopierPackager.h"
#include "ZipperPackager.h"


StormPackager::PackagerManager::PackagerManager() :
	_prepared{ false }
{
	_packagerList.reserve(3);
	_packagerList.emplace_back(std::make_unique<StormPackager::ValidaterPackager>());
	_packagerList.emplace_back(std::make_unique<StormPackager::CopierPackager>());
	_packagerList.emplace_back(std::make_unique<StormPackager::ZipperPackager>());
}

StormPackager::PackagerManager::~PackagerManager() = default;

void StormPackager::PackagerManager::initialize_Implementation()
{
	LOG_COMMENT << "Preparing packaging";

	std::string errorMsg;
	for (auto &packagerLogic : _packagerList)
	{
		errorMsg = packagerLogic->prepare();
		if (!errorMsg.empty())
		{
			LOG_ERROR << "Packager " << packagerLogic->getName() << " logic failed to prepare. We will abort packaging. Error was " << errorMsg;
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
	errorMsgList.reserve(_packagerList.size());

	for (auto &packagerLogic : _packagerList)
	{
		std::string errorMsg = packagerLogic->cleanUp();
		if (!errorMsg.empty())
		{
			errorMsgList.emplace_back(packagerLogic->getName(), std::move(errorMsg));
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
	for (auto &packagerLogic : _packagerList)
	{
		errorMsg = packagerLogic->execute();
		if (!errorMsg.empty())
		{
			LOG_ERROR << "Packager " << packagerLogic->getName() << " logic failed to execute. We will abort packaging. Error was " << errorMsg;
			return false;
		}
	}

	LOG_COMMENT << "Packaging run successfully";

	return true;
}
