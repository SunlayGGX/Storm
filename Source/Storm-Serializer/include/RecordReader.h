#pragma once

#include "RecordHandlerBase.h"


namespace Storm
{
	struct SerializeRecordPendingData;
	class RecordPreHeaderSerializer;

	class RecordReader : public Storm::RecordHandlerBase
	{
	private:
		using ReadMethodDelegate = bool(RecordReader::*)(Storm::SerializeRecordPendingData &) const;

	public:
		RecordReader();
		~RecordReader();

	public:
		bool readNextFrame(Storm::SerializeRecordPendingData &outPendingData) const;

	private:
		bool readNextFrame_v1_0_0(Storm::SerializeRecordPendingData &outPendingData) const;

	public:
		std::unique_ptr<Storm::RecordPreHeaderSerializer> _preheaderSerializer;
		ReadMethodDelegate _readMethodToUse;
	};
}
