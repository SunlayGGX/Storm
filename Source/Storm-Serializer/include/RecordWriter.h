#pragma once

#include "RecordHandlerBase.h"


namespace Storm
{
	struct SerializeRecordPendingData;

	class RecordWriter : public Storm::RecordHandlerBase
	{
	public:
		RecordWriter(Storm::SerializeRecordHeader &&header);

	public:
		void write(const Storm::SerializeRecordPendingData &data);

	private:
		std::size_t _frameNumber;
	};
}
