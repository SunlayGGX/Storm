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
		RecordHandlerBase(Storm::SerializeRecordHeader &&header, const Storm::Version &recordVersion);

	public:
		virtual ~RecordHandlerBase();

	protected:
		virtual void fillSupportedFeature(const Storm::Version &currentVersion, Storm::SerializeSupportedFeatureLayout &missingFeatures) const;

	public:
		void serializeHeader();
		const Storm::SerializeRecordHeader& getHeader() const noexcept;

		void endWriteHeader(uint64_t headerPos, uint64_t frameCount, float realPhysicsTime);

	protected:
		Storm::SerializePackage _package;
		
		std::unique_ptr<Storm::RecordPreHeaderSerializer> _preheaderSerializer;

		Storm::SerializeRecordHeader _header;

		std::size_t _movingSystemCount;
	};
}
