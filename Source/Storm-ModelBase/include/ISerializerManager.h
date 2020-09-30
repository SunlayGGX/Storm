#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	struct SerializeRecordPendingData;

	class ISerializerManager : public Storm::ISingletonHeldInterface<Storm::ISerializerManager>
	{
	public:
		virtual ~ISerializerManager() = default;

	public:
		virtual void recordFrame(Storm::SerializeRecordPendingData &&frameRecord) = 0;
	};
}
