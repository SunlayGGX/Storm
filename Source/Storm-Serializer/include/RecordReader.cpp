#include "RecordReader.h"

#include "RecordPreHeaderSerializer.h"

#include "SerializeRecordHeader.h"
#include "SerializeConstraintLayout.h"
#include "SerializeParticleSystemLayout.h"

#include "SerializeRecordContraintsData.h"
#include "SerializeRecordParticleSystemData.h"
#include "SerializeRecordPendingData.h"

#include "ThrowException.h"

#define STORM_HIJACKED_TYPE Storm::Vector3
#include "VectHijack.h"


Storm::RecordReader::RecordReader() :
	Storm::RecordHandlerBase{ Storm::SerializeRecordHeader{} }
{
	const Storm::Version &currentRecordVersion = _preheaderSerializer->getRecordVersion();
	if (currentRecordVersion < Storm::Version{ 1, 1, 0 })
	{
		_readMethodToUse = &Storm::RecordReader::readNextFrame_v1_0_0;
	}
	else if (currentRecordVersion == Storm::Version{ 1, 1, 0 })
	{
		_readMethodToUse = &Storm::RecordReader::readNextFrame_v1_1_0;
	}
	else if (currentRecordVersion == Storm::Version{ 1, 2, 0 })
	{
		_readMethodToUse = &Storm::RecordReader::readNextFrame_v1_2_0;
	}
	else
	{
		Storm::throwException<std::exception>("Cannot read the current record because the version " + Storm::toStdString(currentRecordVersion) + " isn't handled ");
	}

	Storm::RecordHandlerBase::serializeHeader();
	_firstFramePosition = _package.getStreamPosition();

	_noMoreFrame = _header._frameCount == 0;
}

Storm::RecordReader::~RecordReader() = default;

bool Storm::RecordReader::resetToBeginning()
{
	_noMoreFrame = _header._frameCount == 0;
	const bool hasFrame = !_noMoreFrame;
	
	if (hasFrame)
	{
		_package.seekAbsolute(_firstFramePosition);
	}

	return hasFrame;
}

bool Storm::RecordReader::readNextFrame(Storm::SerializeRecordPendingData &outPendingData)
{
	if (!_noMoreFrame)
	{
		return (this->*_readMethodToUse)(outPendingData);
	}

	return false;
}

bool Storm::RecordReader::readNextFrame_v1_0_0(Storm::SerializeRecordPendingData &outPendingData)
{
	uint64_t frameNumber = std::numeric_limits<uint64_t>::max();
	_package << frameNumber;

	_noMoreFrame = frameNumber >= _header._frameCount;
	if (_noMoreFrame)
	{
		return false;
	}

	_package << outPendingData._physicsTime;

	if (frameNumber == 0)
	{
		// If it is the first frame, then we would have all particles from all rigid bodies (static rigid bodies included).
		outPendingData._particleSystemElements.resize(_header._particleSystemLayouts.size());
	}
	else
	{
		// The other frame have only the particle system that are allowed to move (gain some spaces).
		outPendingData._particleSystemElements.resize(_movingSystemCount);
	}

	for (Storm::SerializeRecordParticleSystemData &frameData : outPendingData._particleSystemElements)
	{
		_package <<
			frameData._systemId <<
			frameData._positions <<
			frameData._velocities <<
			frameData._forces <<
			frameData._pressureComponentforces <<
			frameData._viscosityComponentforces
			;
	}

	return true;
}

bool Storm::RecordReader::readNextFrame_v1_1_0(Storm::SerializeRecordPendingData &outPendingData)
{
	uint64_t frameNumber = std::numeric_limits<uint64_t>::max();
	_package << frameNumber;

	_noMoreFrame = frameNumber >= _header._frameCount;
	if (_noMoreFrame)
	{
		return false;
	}

	_package << outPendingData._physicsTime;

	if (frameNumber == 0)
	{
		// If it is the first frame, then we would have all particles from all rigid bodies (static rigid bodies included).
		outPendingData._particleSystemElements.resize(_header._particleSystemLayouts.size());
	}
	else
	{
		// The other frame have only the particle system that are allowed to move (gain some spaces).
		outPendingData._particleSystemElements.resize(_movingSystemCount);
	}

	for (Storm::SerializeRecordParticleSystemData &frameData : outPendingData._particleSystemElements)
	{
		_package <<
			frameData._systemId <<
			frameData._positions <<
			frameData._velocities <<
			frameData._forces <<
			frameData._pressureComponentforces <<
			frameData._viscosityComponentforces
			;
	}

	outPendingData._constraintElements.resize(_header._contraintLayouts.size());
	for (Storm::SerializeRecordContraintsData &constraintData : outPendingData._constraintElements)
	{
		_package <<
			constraintData._id <<
			constraintData._position1 <<
			constraintData._position2
			;
	}

	return true;
}

bool Storm::RecordReader::readNextFrame_v1_2_0(Storm::SerializeRecordPendingData &outPendingData)
{
	uint64_t frameNumber = std::numeric_limits<uint64_t>::max();
	_package << frameNumber;

	_noMoreFrame = frameNumber >= _header._frameCount;
	if (_noMoreFrame)
	{
		return false;
	}

	_package << outPendingData._physicsTime;

	if (frameNumber == 0)
	{
		// If it is the first frame, then we would have all particles from all rigid bodies (static rigid bodies included).
		outPendingData._particleSystemElements.resize(_header._particleSystemLayouts.size());
	}
	else
	{
		// The other frame have only the particle system that are allowed to move (gain some spaces).
		outPendingData._particleSystemElements.resize(_movingSystemCount);
	}

	for (Storm::SerializeRecordParticleSystemData &frameData : outPendingData._particleSystemElements)
	{
		_package <<
			frameData._systemId <<
			frameData._pSystemPosition <<
			frameData._pSystemGlobalForce <<
			frameData._positions <<
			frameData._velocities <<
			frameData._forces <<
			frameData._pressureComponentforces <<
			frameData._viscosityComponentforces
			;
	}

	outPendingData._constraintElements.resize(_header._contraintLayouts.size());
	for (Storm::SerializeRecordContraintsData &constraintData : outPendingData._constraintElements)
	{
		_package <<
			constraintData._id <<
			constraintData._position1 <<
			constraintData._position2
			;
	}

	return true;
}
