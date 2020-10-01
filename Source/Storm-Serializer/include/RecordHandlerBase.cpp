#include "RecordHandlerBase.h"

#include "ThrowException.h"


namespace
{
	inline std::string retrieveRecordFilePath()
	{
		STORM_NOT_IMPLEMENTED;
	}
}


Storm::RecordHandlerBase::RecordHandlerBase(Storm::SerializeRecordHeader &&header, Storm::SerializePackageCreationModality packageCreationModality) :
	_header{ std::move(header) },
	_package{ packageCreationModality, retrieveRecordFilePath() }
{

}

