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

		// The SerializerManager keep the ownership of the header. But it shouldn't change after the first call to this method, therefore it is ok to share among other threads.
		virtual void beginReplay(const Storm::SerializeRecordHeader* &outRecordHeaderPtr) = 0;

		// Ownership is given to the caller code. Return false if there is no more frame to get.
		virtual bool obtainNextFrame(Storm::SerializeRecordPendingData &outPendingData) const = 0;
	};
}
