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

		void endWrite(float realEndPhysicsTime);

		void flush();

	private:
		void ensureFrameDataCoherency(const Storm::SerializeRecordParticleSystemData &frameData) const;
		void ensureConstraintDataCoherency(const Storm::SerializeRecordContraintsData &constraintData) const;

	private:
		uint64_t _headerPosition;
		uint64_t _recordBodyPosition;

		// This frame number is not the real frame number of the simulation, but more like the recorded frame number
		// those purpose is to track how much frame we recorded so far and to know how much more frame is remaining when we'll read the record.
		uint64_t _frameNumber;
	};
}
