#include "PackagerManager.h"

#include "IPackagingLogic.h"
#include "ValidaterPackager.h"
#include "CopierPackager.h"
#include "ZipperPackager.h"

#include <iostream>


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
	std::cout << "Preparing packaging\n";

	std::string errorMsg;
	for (auto &packagerLogic : _packagerList)
	{
		errorMsg = packagerLogic->prepare();
		if (!errorMsg.empty())
		{
			std::cerr << "Packager " << packagerLogic->getName() << " logic failed to prepare. We will abort packaging. Error was " << errorMsg << ".\n";
			return;
		}
	}

	_prepared = true;
}

void StormPackager::PackagerManager::cleanUp_Implementation()
{
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
			std::cerr << "Packager " << errorInfo.first << " logic failed to cleanup. Error was " << errorInfo.second << ".\n";
		}
	}
}

bool StormPackager::PackagerManager::run()
{
	if (!_prepared)
	{
		return false;
	}

	std::string errorMsg;
	for (auto &packagerLogic : _packagerList)
	{
		errorMsg = packagerLogic->execute();
		if (!errorMsg.empty())
		{
			std::cerr << "Packager " << packagerLogic->getName() << " logic failed to execute. We will abort packaging. Error was " << errorMsg << "\n";
			return false;
		}
	}

	return true;
}
