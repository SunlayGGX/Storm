#include "RecordReader.h"

#include "SerializeRecordHeader.h"
#include "RecordPreHeaderSerializer.h"
#include "SerializeRecordPendingData.h"

#include "ThrowException.h"

#define STORM_HIJACKED_TYPE Storm::Vector3
#include "VectHijack.h"


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

	_noMoreFrame = _header._frameCount == 0;
}

Storm::RecordReader::~RecordReader() = default;

bool Storm::RecordReader::readNextFrame(Storm::SerializeRecordPendingData &outPendingData)
{
	return (this->*_readMethodToUse)(outPendingData);
}

bool Storm::RecordReader::readNextFrame_v1_0_0(Storm::SerializeRecordPendingData &outPendingData)
{
	if (_noMoreFrame)
	{
		return false;
	}

	uint64_t frameNumber;

	_package <<
		frameNumber <<
		outPendingData._physicsTime
		;

	_noMoreFrame = frameNumber == _header._frameCount;

	outPendingData._elements.resize(_header._particleSystemLayouts.size());

	for (Storm::SerializeRecordElementsData &frameData : outPendingData._elements)
	{
		_package <<
			frameData._systemId <<
			frameData._positions <<
			frameData._velocities <<
			frameData._forces
			;
	}

	return true;
}
