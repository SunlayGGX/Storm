#include "RecordReader.h"

#include "SerializeRecordHeader.h"
#include "RecordPreHeaderSerializer.h"

#include "ThrowException.h"


Storm::RecordReader::RecordReader() :
	Storm::RecordHandlerBase{ Storm::SerializeRecordHeader{}, Storm::SerializePackageCreationModality::Loading },
	_preheaderSerializer{ std::make_unique<Storm::RecordPreHeaderSerializer>() }
{
	_package << *_preheaderSerializer;

	const Storm::Version &currentRecordVersion = _preheaderSerializer->getRecordVersion();
	if (currentRecordVersion <= Storm::Version{ 1, 0, 0 })
	{
		_readMethodToUse = &Storm::RecordReader::readNextFrame_v1_0_0;
	}
	else
	{
		Storm::throwException<std::exception>("Cannot read the current record because the version " + Storm::toStdString(currentRecordVersion) + " isn't handled ");
	}

	Storm::RecordHandlerBase::serializeHeader();
}

Storm::RecordReader::~RecordReader() = default;

bool Storm::RecordReader::readNextFrame(Storm::SerializeRecordPendingData &outPendingData)
{
	return (this->*_readMethodToUse)(outPendingData);
}

bool Storm::RecordReader::readNextFrame_v1_0_0(Storm::SerializeRecordPendingData &outPendingData)
{
	STORM_NOT_IMPLEMENTED;
}
