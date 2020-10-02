#include "RecordReader.h"


Storm::RecordReader::RecordReader(Storm::SerializeRecordHeader &&header) :
	Storm::RecordHandlerBase{ std::move(header), Storm::SerializePackageCreationModality::Loading }
{

}
