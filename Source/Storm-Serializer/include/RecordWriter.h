#pragma once

#include "RecordHandlerBase.h"


namespace Storm
{
	struct SerializeRecordPendingData;
	struct SerializeRecordElementsData;
	class RecordPreHeaderSerializer;

	class RecordWriter : public Storm::RecordHandlerBase
	{
	public:
		RecordWriter(Storm::SerializeRecordHeader &&header);
		~RecordWriter();

	public:
		void write(/*const*/ Storm::SerializeRecordPendingData &data);

		void ensureFrameDataCoherency(const Storm::SerializeRecordElementsData &frameData);

		void endWrite();

		void flush();

	private:
		uint64_t _headerPosition;
		uint64_t _recordBodyPosition;

		std::unique_ptr<Storm::RecordPreHeaderSerializer> _preheaderSerializer;

		uint64_t _frameNumber;
	};
}
