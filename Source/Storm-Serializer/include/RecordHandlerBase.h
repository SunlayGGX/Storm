#pragma once

#include "SerializePackage.h"
#include "SerializeRecordHeader.h"


namespace Storm
{
	class RecordPreHeaderSerializer;
	class Version;

	class RecordHandlerBase
	{
	protected:
		// Reading
		RecordHandlerBase(Storm::SerializeRecordHeader &&header);

		// Writing
		RecordHandlerBase(Storm::SerializeRecordHeader &&header, Storm::Version &recordVersion);

	public:
		virtual ~RecordHandlerBase();

	public:
		void serializeHeader();
		const Storm::SerializeRecordHeader& getHeader() const noexcept;

		void endWriteHeader(uint64_t headerPos, uint64_t frameCount);

	protected:
		Storm::SerializePackage _package;
		
		std::unique_ptr<Storm::RecordPreHeaderSerializer> _preheaderSerializer;

		Storm::SerializeRecordHeader _header;

		std::size_t _movingSystemCount;
	};
}
