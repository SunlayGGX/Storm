#pragma once

#include "RecordPreHeader.h"


namespace Storm
{
	class Version;
	class SerializePackage;

	class RecordPreHeaderSerializer
	{
	public:
		// Read
		RecordPreHeaderSerializer();

		// Write
		RecordPreHeaderSerializer(const Storm::Version &version);

	public:
		void serialize(Storm::SerializePackage &package);

		void endSerializing(Storm::SerializePackage &package);

	public:
		const Storm::Version& getRecordVersion() const noexcept;

	private:
		Storm::RecordPreHeader _preHeader;
	};
}
