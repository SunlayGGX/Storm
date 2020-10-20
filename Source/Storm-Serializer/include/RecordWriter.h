#pragma once

#include "RecordHandlerBase.h"


namespace Storm
{
	struct SerializeRecordPendingData;
	struct SerializeRecordParticleSystemData;
	struct SerializeRecordContraintsData;
	class RecordPreHeaderSerializer;

	class RecordWriter : public Storm::RecordHandlerBase
	{
	public:
		RecordWriter(Storm::SerializeRecordHeader &&header);
		~RecordWriter();

	public:
		void write(/*const*/ Storm::SerializeRecordPendingData &data);

		void endWrite();

		void flush();

	private:
		void ensureFrameDataCoherency(const Storm::SerializeRecordParticleSystemData &frameData) const;
		void ensureConstraintDataCoherency(const Storm::SerializeRecordContraintsData &constraintData) const;

	private:
		uint64_t _headerPosition;
		uint64_t _recordBodyPosition;

		uint64_t _frameNumber;
	};
}
