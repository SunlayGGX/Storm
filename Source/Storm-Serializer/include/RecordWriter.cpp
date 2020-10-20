#include "RecordWriter.h"

#include "SerializeRecordContraintsData.h"
#include "SerializeRecordParticleSystemData.h"
#include "SerializeRecordPendingData.h"

#include "SerializeParticleSystemLayout.h"
#include "SerializeConstraintLayout.h"

#include "Version.h"

#include "ThrowException.h"


namespace
{
	// This is the version for the current writer.
	// The application will always write the latest version, therefore it is useless to maintain different record version for the writer.
	// Each time you change/add/remove something that modifies the layout of the recording, increase the version number here (to not break the retro compatibility). 
	constexpr Storm::Version retrieveRecordPacketVersion()
	{
		return Storm::Version{ 1, 1, 0 };
	}

	void recordStreamPosition(Storm::RecordWriter*const recordWriter, uint64_t &outPosition, const std::filesystem::path &recordFilePath)
	{
		recordWriter->flush();
		outPosition = std::filesystem::file_size(recordFilePath);
	}
}


Storm::RecordWriter::RecordWriter(Storm::SerializeRecordHeader &&header) :
	Storm::RecordHandlerBase{ std::move(header), retrieveRecordPacketVersion() },
	_frameNumber{ 0 }
{
	const std::filesystem::path recordFilePath = _package.getFilePath();

	recordStreamPosition(this, _headerPosition, recordFilePath);

	Storm::RecordHandlerBase::serializeHeader();

	recordStreamPosition(this, _recordBodyPosition, recordFilePath);
}

Storm::RecordWriter::~RecordWriter() = default;

void Storm::RecordWriter::write(/*const*/ Storm::SerializeRecordPendingData &data)
{
	if (!_preheaderSerializer)
	{
		Storm::throwException<std::exception>("We cannot write after we have ended the write!");
	}

	const std::size_t dataElementCount = data._particleSystemElements.size();
	if (!(
		(_frameNumber > 0 && dataElementCount == _movingSystemCount) ||
		(_frameNumber == 0 && dataElementCount == _header._particleSystemLayouts.size())
		))
	{
		Storm::throwException<std::exception>(
			"Frame particle system layout of what to be recorded doesn't match the frame data.\n"
			"We should have " + std::to_string(_header._particleSystemLayouts.size()) + " particle systems.\n"
			"But for the frame (" + std::to_string(_frameNumber) + "), we have " + std::to_string(dataElementCount) + " particle systems."
		);
	}
	const std::size_t constraintElementCount = data._constraintElements.size();
	if (constraintElementCount != _header._contraintLayouts.size())
	{
		Storm::throwException<std::exception>(
			"Frame constraint layout of what to be recorded doesn't match the frame data.\n"
			"We should have " + std::to_string(_header._contraintLayouts.size()) + " constraints.\n"
			"But for the frame (" + std::to_string(_frameNumber) + "), we have " + std::to_string(constraintElementCount) + " constraints."
		);
	}

	_package <<
		_frameNumber <<
		data._physicsTime
		;

	for (Storm::SerializeRecordParticleSystemData &frameData : data._particleSystemElements)
	{
		this->ensureFrameDataCoherency(frameData);

		_package <<
			frameData._systemId <<
			frameData._positions <<
			frameData._velocities <<
			frameData._forces <<
			frameData._pressureComponentforces <<
			frameData._viscosityComponentforces
			;
	}

	for (Storm::SerializeRecordContraintsData &constraintData : data._constraintElements)
	{
		this->ensureConstraintDataCoherency(constraintData);

		_package <<
			constraintData._id <<
			constraintData._position1 <<
			constraintData._position2
			;
	}

	LOG_DEBUG << "Frame " << _frameNumber << " recorded.";

	++_frameNumber;
}

void Storm::RecordWriter::endWrite()
{
	Storm::RecordHandlerBase::endWriteHeader(_headerPosition, _frameNumber);

	this->flush();
}

void Storm::RecordWriter::flush()
{
	_package.flush();
}

void Storm::RecordWriter::ensureFrameDataCoherency(const Storm::SerializeRecordParticleSystemData &frameData) const
{
	const std::size_t positionsCount = frameData._positions.size();
	const std::size_t velocitiesCount = frameData._velocities.size();
	const std::size_t forcesCount = frameData._forces.size();
	const std::size_t pressureTmpForceCount = frameData._pressureComponentforces.size();
	const std::size_t viscosityTmpForceCount = frameData._viscosityComponentforces.size();
	if (
		positionsCount != velocitiesCount ||
		positionsCount != forcesCount ||
		positionsCount != pressureTmpForceCount ||
		positionsCount != viscosityTmpForceCount
		)
	{
		Storm::throwException<std::exception>("Frame " + std::to_string(_frameNumber) + " data particle count mismatches!");
	}

	const auto endHeaderParticleSystemLayout = std::end(_header._particleSystemLayouts);
	const auto associatedLayout = std::find_if(std::begin(_header._particleSystemLayouts), endHeaderParticleSystemLayout, [sysId = frameData._systemId](const Storm::SerializeParticleSystemLayout &layout)
	{
		return layout._particleSystemId == sysId;
	});

	if (associatedLayout == endHeaderParticleSystemLayout)
	{
		Storm::throwException<std::exception>("Frame " + std::to_string(_frameNumber) + ": Unknown frame system id to record (" + std::to_string(frameData._systemId) + ")");
	}

	if (associatedLayout->_particlesCount != positionsCount)
	{
		Storm::throwException<std::exception>("Frame " + std::to_string(_frameNumber) + ", particle system " + std::to_string(frameData._systemId) + ": The particle count to record does not match what was registered as layout\n"
			"We should have " + std::to_string(associatedLayout->_particlesCount) + " particles.\n"
			"This frame contains " + std::to_string(positionsCount) + " particles.\n"
			"Note that we don't support particles added to simulation when recording.");
	}
}

void Storm::RecordWriter::ensureConstraintDataCoherency(const Storm::SerializeRecordContraintsData &constraintData) const
{
	const auto endHeaderConstraintsLayout = std::end(_header._contraintLayouts);
	const auto associatedLayout = std::find_if(std::begin(_header._contraintLayouts), endHeaderConstraintsLayout, [sysId = constraintData._id](const Storm::SerializeConstraintLayout &layout)
	{
		return layout._id == sysId;
	});

	if (associatedLayout == endHeaderConstraintsLayout)
	{
		Storm::throwException<std::exception>("Frame " + std::to_string(_frameNumber) + ": Unknown frame constraint id to record (" + std::to_string(constraintData._id) + ")");
	}
}

