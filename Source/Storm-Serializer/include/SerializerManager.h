#pragma once

#include "Singleton.h"
#include "ISerializerManager.h"


namespace Storm
{
	class RecordReader;
	class RecordWriter;
	class RecordArchiver;

	class SerializerManager final :
		private Storm::Singleton<Storm::SerializerManager>,
		public Storm::ISerializerManager
	{
		STORM_DECLARE_SINGLETON(SerializerManager);

	public:
		struct ExporterToolTag{};

	private:
		void initialize_Implementation();
		void cleanUp_Implementation();

		void initialize_Implementation(ExporterToolTag);
		void cleanUp_Implementation(ExporterToolTag);

	private:
		void run();
		void execute();

	private:
		void clearRecordQueue();
		void processRecordQueue_Unchecked();

	public:
		void recordFrame(Storm::SerializeRecordPendingData &&frameRecord) final override;
		void beginRecord(Storm::SerializeRecordHeader &&recordHeader) final override;

	private:
		void endRecordInternal();

	public:
		const Storm::SerializeRecordHeader& beginReplay() final override;
		bool obtainNextFrame(Storm::SerializeRecordPendingData &outPendingData) const final override;

		const Storm::SerializeRecordHeader& getRecordHeader() const final override;

		std::shared_ptr<Storm::SerializeSupportedFeatureLayout> getRecordSupportedFeature() const final override;

	public:
		bool resetReplay() final override;

	public:
		void saveState(Storm::StateSavingOrders &&savingOrder) final override;
		void loadState(Storm::StateLoadingOrders &inOutLoadingOrder) final override;

	public:
		std::string getArchivePath() const final override;

	public:
		void exportRecord(const std::string &recordFile, const ExporterEventCallbacks &exporter) final override;

	private:
		void checkExporterCallbackValidity(const ExporterEventCallbacks &exporter) const;

	private:
		std::thread _serializeThread;

		// Recorder
		std::unique_ptr<Storm::RecordReader> _recordReader;
		std::unique_ptr<Storm::RecordWriter> _recordWriter;
		std::queue<std::unique_ptr<Storm::SerializeRecordPendingData>> _pendingRecord;

		// State recording
		std::unique_ptr<Storm::StateSavingOrders> _stateSavingRequestOrders;

		// Archiver
		std::unique_ptr<Storm::RecordArchiver> _archiver;
		std::string _archivePathCachedNoCleanUp; // This must survive cleanUp

		// Exporter
		bool _initForExport;

		mutable std::mutex _mutex;
	};
}
