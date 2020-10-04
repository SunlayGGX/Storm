#include "RecordWriter.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

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
}


Storm::RecordWriter::RecordWriter(Storm::SerializeRecordHeader &&header) :
	Storm::RecordHandlerBase{ std::move(header), Storm::SerializePackageCreationModality::SavingAppendPreheaderProvidedAfter },
	_frameNumber{ 0 }
{
}

void Storm::RecordWriter::write(/*const*/ Storm::SerializeRecordPendingData &data)
{
	STORM_NOT_IMPLEMENTED;
}

