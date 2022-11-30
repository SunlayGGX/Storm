#include "PatioWriter.h"

#include "IExporterConfigManager.h"
#include "SingletonHolder.h"

#include "Vector3.h"

#include "SerializeParticleSystemLayout.h"
#include "SerializeRecordHeader.h"
#include "SerializeRecordParticleSystemData.h"
#include "SerializeRecordPendingData.h"

#include "ExportMode.h"


StormExporter::PatioWriter::PatioWriter(const Storm::SerializeRecordHeader &header)
{
	LOG_COMMENT << "Record header parsed. We'll start writing to Patio.";
	
	const auto &configMgr = Storm::SingletonHolder::instance().getSingleton<StormExporter::IExporterConfigManager>();

	const std::string &outFile = configMgr.getOutExportPath();
	_patioFile.open(outFile, std::ios_base::out | std::ios_base::binary);
	if (!_patioFile.is_open())
	{
		Storm::throwException<Storm::Exception>("Cannot open file " + outFile + " to write into!");
	}

	const StormExporter::ExportMode mode = configMgr.getExportMode();

	const bool targetFluids = STORM_IS_BIT_ENABLED(mode, StormExporter::ExportMode::Fluid);
	const bool targetRigidBodies = STORM_IS_BIT_ENABLED(mode, StormExporter::ExportMode::RigidBody);

	for (const auto &layout : header._particleSystemLayouts)
	{
		if (layout._particlesCount > 0 && 
			(
				(targetFluids && layout._isFluid) ||
				(targetRigidBodies && !layout._isFluid && !layout._isStatic)
			))
		{
			_targetIds.emplace_back(layout._particleSystemId);
		}
	}

	if (_targetIds.empty())
	{
		Storm::throwException<Storm::Exception>("No valid particle system exists inside the record file!");
	}
}

StormExporter::PatioWriter::~PatioWriter() = default;

bool StormExporter::PatioWriter::onFrameExport(const Storm::SerializeRecordPendingData &frame)
{
	for (const auto &data : frame._particleSystemElements)
	{
		if (this->shouldWriteData(data))
		{
			this->writeData(data);
		}
	}
	return true;
}

void StormExporter::PatioWriter::onExportClose()
{
	LOG_COMMENT << "Finished exporting data to patio file.";
}

bool StormExporter::PatioWriter::shouldWriteData(const Storm::SerializeRecordParticleSystemData &pSystemData) const
{
	return std::any_of(std::begin(_targetIds), std::end(_targetIds), [id = pSystemData._systemId](const unsigned int targetId)
	{
		return targetId == id;
	});
}

void StormExporter::PatioWriter::writeData(const Storm::SerializeRecordParticleSystemData &pSystemData)
{
	const auto &pPositions = pSystemData._positions;
	_patioFile.write(reinterpret_cast<const char*>(pPositions.data()), sizeof(Storm::Vector3) * pPositions.size());
}
