#pragma once

#include "RecordHandlerBase.h"


namespace Storm
{
	struct SerializeRecordPendingData;

	class RecordWriter : public Storm::RecordHandlerBase
	{
	public:
		RecordWriter(Storm::SerializeRecordHeader &&header);
		~RecordWriter();

	public:
		void write(/*const*/ Storm::SerializeRecordPendingData &data);

	private:
		uint64_t _frameNumber;
	};
}
