#pragma once

#include <fstream>

namespace Storm
{
	struct SerializeRecordHeader;
	struct SerializeRecordPendingData;
	struct SerializeRecordParticleSystemData;
}

namespace SlgPWriterPImplDetails
{
	struct SlgPDataWriterBlackboard;
}

namespace StormExporter
{
	class SlgPWriter
	{
	public:
		SlgPWriter(const Storm::SerializeRecordHeader &header);
		~SlgPWriter();

	public:
		bool onFrameExport(const Storm::SerializeRecordPendingData &frame);
		void onExportClose();

	private:
		bool shouldWriteData(const Storm::SerializeRecordParticleSystemData &pSystemData) const;

	private:
		std::vector<unsigned int> _targetIds;
		std::unique_ptr<SlgPWriterPImplDetails::SlgPDataWriterBlackboard> _blackboard;
	};
}
