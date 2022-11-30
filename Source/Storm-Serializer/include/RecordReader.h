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
		RecordReader(const std::string &recordFilePath);
		~RecordReader();

	private:
		void init();

	public:
		bool resetToBeginning();

	public:
		bool readNextFrame(Storm::SerializeRecordPendingData &outPendingData);

	protected:
		void fillSupportedFeature(const Storm::Version &currentVersion, Storm::SerializeSupportedFeatureLayout &missingFeatures) const final override;

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
		bool readNextFrame_v1_7_0(Storm::SerializeRecordPendingData &outPendingData);
		bool readNextFrame_v1_8_0(Storm::SerializeRecordPendingData &outPendingData);
		bool readNextFrame_v1_9_0(Storm::SerializeRecordPendingData &outPendingData);
		bool readNextFrame_v1_10_0(Storm::SerializeRecordPendingData &outPendingData);
		bool readNextFrame_v1_11_0(Storm::SerializeRecordPendingData &outPendingData);
		bool readNextFrame_v1_12_0(Storm::SerializeRecordPendingData &outPendingData);
		bool readNextFrame_v1_13_0(Storm::SerializeRecordPendingData &outPendingData);
		bool readNextFrame_v1_14_0(Storm::SerializeRecordPendingData &outPendingData);

	public:
		ReadMethodDelegate _readMethodToUse;

		bool _noMoreFrame;
		std::size_t _firstFramePosition;
	};
}
