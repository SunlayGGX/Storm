#pragma once

#include <fstream>

namespace Storm
{
	struct SerializeRecordHeader;
	struct SerializeRecordPendingData;
	struct SerializeRecordParticleSystemData;
}

namespace Partio
{
	class ParticlesDataMutable;
}

namespace PartioWriterPImplDetails
{
	struct PartioDataWriterBlackboard;
}

namespace StormExporter
{
	class PartioWriter
	{
	public:
		PartioWriter(const Storm::SerializeRecordHeader &header);
		~PartioWriter();

	public:
		bool onFrameExport(const Storm::SerializeRecordPendingData &frame);
		void onExportClose();

	private:
		bool shouldWriteData(const Storm::SerializeRecordParticleSystemData &pSystemData) const;
		void writeData(const float time, const Storm::SerializeRecordParticleSystemData &pSystemData);

	private:
		std::vector<unsigned int> _targetIds;
		std::shared_ptr<Partio::ParticlesDataMutable> _particleInstance;
		std::unique_ptr<PartioWriterPImplDetails::PartioDataWriterBlackboard> _blackboard;
	};
}
