#include "RecordWriter.h"

#include "RecordPreHeaderSerializer.h"
#include "SerializeRecordPendingData.h"

#include "Version.h"

#include "ThrowException.h"


namespace
{
	// This is the version for the current writer.
	// The application will always write the latest version, therefore it is useless to maintain different record version for the writer.
	// Each time you change/add/remove something that modifies the layout of the recording, increase the version number here (to not break the retro compatibility). 
	constexpr Storm::Version retrieveRecordPacketVersion()
	{
		return Storm::Version{ 1, 0, 0 };
	}

	void recordStreamPosition(Storm::RecordWriter*const recordWriter, uint64_t &outPosition, const std::filesystem::path &recordFilePath)
	{
		recordWriter->flush();
		outPosition = std::filesystem::file_size(recordFilePath);
	}
}


Storm::RecordWriter::RecordWriter(Storm::SerializeRecordHeader &&header) :
	Storm::RecordHandlerBase{ std::move(header), Storm::SerializePackageCreationModality::SavingAppendPreheaderProvidedAfter },
	_preheaderSerializer{ std::make_unique<Storm::RecordPreHeaderSerializer>(retrieveRecordPacketVersion()) },
	_frameNumber{ 0 }
{
	_package << *_preheaderSerializer;

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

	if (data._elements.size() != _header._particleSystemLayouts.size())
	{
		Storm::throwException<std::exception>(
			"Layout of what to be recorded doesn't match the frame data.\n"
			"We should have " + std::to_string(_header._particleSystemLayouts.size()) + " particle systems.\n"
			"But for this frame, we have " + std::to_string(data._elements.size()) + " particle systems."
		);
	}

	_package <<
		_frameNumber <<
		data._physicsTime
		;

	for (Storm::SerializeRecordElementsData &frameData : data._elements)
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

	++_frameNumber;
}

void Storm::RecordWriter::ensureFrameDataCoherency(const Storm::SerializeRecordElementsData &frameData)
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

void Storm::RecordWriter::endWrite()
{
	Storm::RecordHandlerBase::endWriteHeader(_headerPosition, _frameNumber);

	_preheaderSerializer->endSerializing(_package);
	_preheaderSerializer.reset();

	this->flush();
}

void Storm::RecordWriter::flush()
{
	_package.flush();
}

