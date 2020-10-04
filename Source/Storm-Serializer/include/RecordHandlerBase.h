#pragma once

#include "SerializePackage.h"
#include "SerializeRecordHeader.h"


namespace Storm
{
	class RecordHandlerBase
	{
	protected:
		RecordHandlerBase(Storm::SerializeRecordHeader &&header, Storm::SerializePackageCreationModality packageCreationModality);

	public:
		virtual ~RecordHandlerBase() = default;

	public:
		void serializeHeader();

	protected:
		Storm::SerializePackage _package;
		Storm::SerializeRecordHeader _header;
	};
}
