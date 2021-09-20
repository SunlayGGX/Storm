#include "RecordReader.h"

#include "RecordPreHeaderSerializer.h"

#include "SerializeRecordHeader.h"
#include "SerializeConstraintLayout.h"
#include "SerializeParticleSystemLayout.h"

#include "SerializeRecordContraintsData.h"
#include "SerializeRecordParticleSystemData.h"
#include "SerializeRecordPendingData.h"

#define STORM_HIJACKED_TYPE Storm::Vector3
#include "VectHijack.h"


namespace
{
	template<Storm::Version::VersionNumber major, Storm::Version::VersionNumber minor, Storm::Version::VersionNumber subminor>
	void correctVersionMismatchImpl(Storm::SerializeRecordPendingData &outPendingData)
	{
		constexpr Storm::Version currentVersion{ major, minor, subminor };
		for (Storm::SerializeRecordParticleSystemData &frameData : outPendingData._particleSystemElements)
		{
			const std::size_t framePCount = frameData._positions.size();

			if constexpr (currentVersion < Storm::Version{ 1, 2, 0 })
			{
				//frameData._pSystemPosition
				frameData._pSystemGlobalForce = Storm::Vector3::Zero();
			}

			if constexpr (currentVersion < Storm::Version{ 1, 3, 0 })
			{
				frameData._densities.resize(framePCount, 0.f);;
				frameData._pressures.resize(framePCount, 0.f);;
			}

			if constexpr (currentVersion < Storm::Version{ 1, 4, 0 })
			{
				frameData._volumes.resize(framePCount, 0.f);
			}

			if constexpr (currentVersion < Storm::Version{ 1, 5, 0 })
			{
				frameData._dragComponentforces.resize(framePCount, Storm::Vector3::Zero());
			}

			if constexpr (currentVersion < Storm::Version{ 1, 6, 0 })
			{
				frameData._dynamicPressureQForces.resize(framePCount, Storm::Vector3::Zero());
			}

			if constexpr (currentVersion < Storm::Version{ 1, 7, 0 })
			{
				frameData._normals.resize(framePCount, Storm::Vector3::Zero());
				frameData._noStickForces.resize(framePCount, Storm::Vector3::Zero());
			}
		}
	}
}


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
	else if (currentRecordVersion == Storm::Version{ 1, 3, 0 })
	{
		_readMethodToUse = &Storm::RecordReader::readNextFrame_v1_3_0;
	}
	else if (currentRecordVersion == Storm::Version{ 1, 4, 0 })
	{
		_readMethodToUse = &Storm::RecordReader::readNextFrame_v1_4_0;
	}
	else if (currentRecordVersion == Storm::Version{ 1, 5, 0 })
	{
		_readMethodToUse = &Storm::RecordReader::readNextFrame_v1_5_0;
	}
	else if (currentRecordVersion == Storm::Version{ 1, 6, 0 })
	{
		_readMethodToUse = &Storm::RecordReader::readNextFrame_v1_6_0;
	}
	else if (currentRecordVersion == Storm::Version{ 1, 7, 0 })
	{
		_readMethodToUse = &Storm::RecordReader::readNextFrame_v1_7_0;
	}
	else
	{
		Storm::throwException<Storm::Exception>("Cannot read the current record because the version " + Storm::toStdString(currentRecordVersion) + " isn't handled ");
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
		if ((this->*_readMethodToUse)(outPendingData))
		{
			this->correctVersionMismatch(outPendingData);
			return true;
		}
	}

	return false;
}

void Storm::RecordReader::correctVersionMismatch(Storm::SerializeRecordPendingData &outPendingData)
{
	const Storm::Version &currentRecordVersion = _preheaderSerializer->getRecordVersion();
	if (currentRecordVersion < Storm::Version{ 1, 1, 0 })
	{
		correctVersionMismatchImpl<1, 0, 0>(outPendingData);
	}
	else if (currentRecordVersion == Storm::Version{ 1, 1, 0 })
	{
		correctVersionMismatchImpl<1, 1, 0>(outPendingData);
	}
	else if (currentRecordVersion == Storm::Version{ 1, 2, 0 })
	{
		correctVersionMismatchImpl<1, 2, 0>(outPendingData);
	}
	else if (currentRecordVersion == Storm::Version{ 1, 3, 0 })
	{
		correctVersionMismatchImpl<1, 3, 0>(outPendingData);
	}
	else if (currentRecordVersion == Storm::Version{ 1, 4, 0 })
	{
		correctVersionMismatchImpl<1, 4, 0>(outPendingData);
	}
	else if (currentRecordVersion == Storm::Version{ 1, 5, 0 })
	{
		correctVersionMismatchImpl<1, 5, 0>(outPendingData);
	}
	else if (currentRecordVersion == Storm::Version{ 1, 6, 0 })
	{
		correctVersionMismatchImpl<1, 6, 0>(outPendingData);
	}
	else if (currentRecordVersion == Storm::Version{ 1, 7, 0 })
	{
		correctVersionMismatchImpl<1, 7, 0>(outPendingData);
	}
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

bool Storm::RecordReader::readNextFrame_v1_3_0(Storm::SerializeRecordPendingData &outPendingData)
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
			frameData._densities <<
			frameData._pressures <<
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

bool Storm::RecordReader::readNextFrame_v1_4_0(Storm::SerializeRecordPendingData &outPendingData)
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
			frameData._densities <<
			frameData._pressures <<
			frameData._volumes <<
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

bool Storm::RecordReader::readNextFrame_v1_5_0(Storm::SerializeRecordPendingData &outPendingData)
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
			frameData._densities <<
			frameData._pressures <<
			frameData._volumes <<
			frameData._pressureComponentforces <<
			frameData._viscosityComponentforces <<
			frameData._dragComponentforces
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

bool Storm::RecordReader::readNextFrame_v1_6_0(Storm::SerializeRecordPendingData &outPendingData)
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
			frameData._densities <<
			frameData._pressures <<
			frameData._volumes <<
			frameData._pressureComponentforces <<
			frameData._viscosityComponentforces <<
			frameData._dragComponentforces <<
			frameData._dynamicPressureQForces
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

bool Storm::RecordReader::readNextFrame_v1_7_0(Storm::SerializeRecordPendingData& outPendingData)
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
			frameData._densities <<
			frameData._pressures <<
			frameData._volumes <<
			frameData._normals <<
			frameData._pressureComponentforces <<
			frameData._viscosityComponentforces <<
			frameData._dragComponentforces <<
			frameData._dynamicPressureQForces <<
			frameData._noStickForces
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
