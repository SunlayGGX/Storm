#pragma once

#include <fstream>

namespace Storm
{
	struct SerializeRecordHeader;
	struct SerializeRecordPendingData;
	struct SerializeRecordParticleSystemData;
}

namespace StormExporter
{
	class PatioWriter
	{
	public:
		PatioWriter(const Storm::SerializeRecordHeader &header);
		~PatioWriter();

	public:
		bool onFrameExport(const Storm::SerializeRecordPendingData &frame);
		void onExportClose();

	private:
		bool shouldWriteData(const Storm::SerializeRecordParticleSystemData &pSystemData) const;
		void writeData(const Storm::SerializeRecordParticleSystemData &pSystemData);

	private:
		std::vector<unsigned int> _targetIds;
		std::ofstream _patioFile;
	};
}
