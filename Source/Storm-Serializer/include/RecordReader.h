#pragma once

#include "RecordHandlerBase.h"


namespace Storm
{
	struct SerializeRecordPendingData;
	class RecordPreHeaderSerializer;

	class RecordReader : public Storm::RecordHandlerBase
	{
	private:
		using ReadMethodDelegate = bool(RecordReader::*)(Storm::SerializeRecordPendingData &);

	public:
		RecordReader();
		~RecordReader();

	public:
		bool readNextFrame(Storm::SerializeRecordPendingData &outPendingData);

	private:
		bool readNextFrame_v1_0_0(Storm::SerializeRecordPendingData &outPendingData);
		bool readNextFrame_v1_1_0(Storm::SerializeRecordPendingData &outPendingData);

	public:
		ReadMethodDelegate _readMethodToUse;

		bool _noMoreFrame;
	};
}
