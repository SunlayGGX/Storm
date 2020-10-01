#include "RecordWriter.h"


Storm::RecordWriter::RecordWriter(Storm::SerializeRecordHeader &&header) :
	Storm::RecordHandlerBase{ std::move(header), Storm::SerializePackageCreationModality::SavingAppendPreheaderProvidedAfter }
{

}

