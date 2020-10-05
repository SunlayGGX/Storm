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
		const Storm::SerializeRecordHeader& getHeader() const noexcept;

		void endWriteHeader(uint64_t headerPos, uint64_t frameCount);

	protected:
		Storm::SerializePackage _package;
		Storm::SerializeRecordHeader _header;
	};
}
