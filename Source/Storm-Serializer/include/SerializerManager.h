#pragma once

#include "Singleton.h"
#include "ISerializerManager.h"


namespace Storm
{
	class RecordReader;
	class RecordWriter;

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

	public:
		void recordFrame(Storm::SerializeRecordPendingData &&frameRecord) final override;
		void beginRecord(Storm::SerializeRecordHeader &&recordHeader) final override;
		void endRecord() final override;

	public:
		const Storm::SerializeRecordHeader& beginReplay() final override;
		bool obtainNextFrame(Storm::SerializeRecordPendingData &outPendingData) const final override;

	private:
		std::thread _serializeThread;

		// Recorder
		std::unique_ptr<Storm::RecordReader> _recordReader;
		std::unique_ptr<Storm::RecordWriter> _recordWriter;
		std::queue<std::unique_ptr<Storm::SerializeRecordPendingData>> _pendingRecord;

		mutable std::mutex _mutex;
	};
}
