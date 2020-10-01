#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	struct SerializeRecordHeader;
	struct SerializeRecordPendingData;

	class ISerializerManager : public Storm::ISingletonHeldInterface<Storm::ISerializerManager>
	{
	public:
		virtual ~ISerializerManager() = default;

	public:
		virtual void recordFrame(Storm::SerializeRecordPendingData &&frameRecord) = 0;
		virtual void beginRecord(Storm::SerializeRecordHeader &&recordHeader) = 0;
	};
}
