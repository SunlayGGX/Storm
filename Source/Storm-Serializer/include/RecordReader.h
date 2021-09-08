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
		bool resetToBeginning();

	public:
		bool readNextFrame(Storm::SerializeRecordPendingData &outPendingData);

	private:
		void correctVersionMismatch(Storm::SerializeRecordPendingData &outPendingData);

	private:
		bool readNextFrame_v1_0_0(Storm::SerializeRecordPendingData &outPendingData);
		bool readNextFrame_v1_1_0(Storm::SerializeRecordPendingData &outPendingData);
		bool readNextFrame_v1_2_0(Storm::SerializeRecordPendingData &outPendingData);
		bool readNextFrame_v1_3_0(Storm::SerializeRecordPendingData &outPendingData);
		bool readNextFrame_v1_4_0(Storm::SerializeRecordPendingData &outPendingData);
		bool readNextFrame_v1_5_0(Storm::SerializeRecordPendingData &outPendingData);
		bool readNextFrame_v1_6_0(Storm::SerializeRecordPendingData &outPendingData);

	public:
		ReadMethodDelegate _readMethodToUse;

		bool _noMoreFrame;
		std::size_t _firstFramePosition;
	};
}
