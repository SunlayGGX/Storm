#pragma once

#include "Singleton.h"
#include "ISerializerManager.h"


namespace Storm
{
	class SerializerManager :
		private Storm::Singleton<Storm::SerializerManager>,
		public Storm::ISerializerManager
	{
		STORM_DECLARE_SINGLETON(SerializerManager);

	private:
		void initialize_Implementation();
		void cleanUp_Implementation();

	private:
		void run();
		void execute();

	private:
		void clearRecordQueue();
		void processRecordQueue_Unchecked();
		void processRecord(const Storm::SerializeRecordPendingData &record);

	public:
		void recordFrame(Storm::SerializeRecordPendingData &&frameRecord) final override;

	private:
		std::thread _serializeThread;

		std::queue<std::unique_ptr<Storm::SerializeRecordPendingData>> _pendingRecord;

		bool _hasRecordHeader;
	};
}
