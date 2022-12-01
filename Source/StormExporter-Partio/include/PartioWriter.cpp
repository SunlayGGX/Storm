#include "PartioWriter.h"

#include "IExporterConfigManager.h"
#include "SingletonHolder.h"

#include "Vector3.h"

#include "SerializeParticleSystemLayout.h"
#include "SerializeRecordHeader.h"
#include "SerializeRecordParticleSystemData.h"
#include "SerializeRecordPendingData.h"

#include "ExportMode.h"

#include <Partio.h>


namespace PartioWriterPImplDetails
{
	struct PartioDataWriterBlackboard
	{
	public:
		void init(Partio::ParticlesDataMutable &data)
		{
			_position = data.addAttribute("position", Partio::VECTOR, 3);
			_time = data.addAttribute("time", Partio::FLOAT, 1);
			_id = data.addAttribute("id", Partio::INT, 1);
		}

	public:
		Partio::ParticleAttribute _id;
		Partio::ParticleAttribute _time;
		Partio::ParticleAttribute _position;
	};
}


StormExporter::PartioWriter::PartioWriter(const Storm::SerializeRecordHeader &header) :
	_particleInstance{ Partio::create(), [](auto* instance) { instance->release(); } },
	_blackboard{ std::make_unique<std::remove_cvref_t<decltype(*_blackboard)>>() }
{
	LOG_COMMENT << "Record header parsed. We'll start writing to Partio.";

	_blackboard->init(*_particleInstance);

	const auto &configMgr = Storm::SingletonHolder::instance().getSingleton<StormExporter::IExporterConfigManager>();
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

StormExporter::PartioWriter::~PartioWriter() = default;

bool StormExporter::PartioWriter::onFrameExport(const Storm::SerializeRecordPendingData &frame)
{
	for (const auto &data : frame._particleSystemElements)
	{
		if (this->shouldWriteData(data))
		{
			this->writeData(frame._physicsTime, data);
		}
	}
	return true;
}

void StormExporter::PartioWriter::onExportClose()
{
	const auto &configMgr = Storm::SingletonHolder::instance().getSingleton<StormExporter::IExporterConfigManager>();
	const std::string &fileToExport = configMgr.getOutExportPath();

	LOG_COMMENT << "Finished transferring data to partio.\nWriting partio file at '" << fileToExport << '\'';

	std::stringstream errorStreamRedirect;
	Partio::write(fileToExport.c_str(), *_particleInstance, false, true, errorStreamRedirect);

	if (const std::string_view errorMsg = errorStreamRedirect.view();
		!errorMsg.empty())
	{
		LOG_ERROR << "Error when writing partio file :\n" << std::move(errorStreamRedirect).str();
	}
}

bool StormExporter::PartioWriter::shouldWriteData(const Storm::SerializeRecordParticleSystemData &pSystemData) const
{
	return std::any_of(std::begin(_targetIds), std::end(_targetIds), [id = pSystemData._systemId](const unsigned int targetId)
	{
		return targetId == id;
	});
}

void StormExporter::PartioWriter::writeData(const float time, const Storm::SerializeRecordParticleSystemData &pSystemData)
{
	const std::vector<Storm::Vector3> &pPositions = pSystemData._positions;
	const std::size_t particleCount = pPositions.size();

	auto pIterator = _particleInstance->addParticles(static_cast<int>(particleCount));

	Partio::ParticleAccessor idAccessor{ _blackboard->_id };
	pIterator.addAccessor(idAccessor);

	Partio::ParticleAccessor timeAccessor{ _blackboard->_time };
	pIterator.addAccessor(timeAccessor);
	
	Partio::ParticleAccessor positionAccessor{ _blackboard->_position };
	pIterator.addAccessor(positionAccessor);

	for (std::size_t iter = 0; iter < particleCount; ++iter)
	{
		*idAccessor.raw<int>(pIterator) = pSystemData._systemId;
		*timeAccessor.raw<float>(pIterator) = time;
		memcpy(positionAccessor.raw<float>(pIterator), &pPositions[iter], sizeof(Storm::Vector3));

		++pIterator;
	}
}
