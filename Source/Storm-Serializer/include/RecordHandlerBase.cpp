#include "RecordHandlerBase.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "ThrowException.h"


namespace
{
	inline std::string retrieveRecordFilePath()
	{
		const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
		return configMgr.getRecordConfigData()._recordFilePath;
	}
}


Storm::RecordHandlerBase::RecordHandlerBase(Storm::SerializeRecordHeader &&header, Storm::SerializePackageCreationModality packageCreationModality) :
	_header{ std::move(header) },
	_package{ packageCreationModality, retrieveRecordFilePath() }
{

}

