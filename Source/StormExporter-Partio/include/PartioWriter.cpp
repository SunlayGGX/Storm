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
			//_time = data.addAttribute("time", Partio::FLOAT, 1);
			_position = data.addAttribute("position", Partio::VECTOR, 3);
			_velocity = data.addAttribute("velocity", Partio::VECTOR, 3);
			_id = data.addAttribute("id", Partio::INT, 1);
		}

	public:
		Partio::ParticleAttribute _id;
		//Partio::ParticleAttribute _time;
		Partio::ParticleAttribute _position;
		Partio::ParticleAttribute _velocity;
	};
}

namespace
{
	Partio::ParticlesDataMutable::iterator resizeForThisFrame(Partio::ParticlesDataMutable &instance, const Storm::SerializeRecordPendingData &frame)
	{
		Partio::ParticlesDataMutable::iterator pIterator;

		std::size_t particleCountThisFrame = 0;
		bool firstAdd = false;
		for (const auto &data : frame._particleSystemElements)
		{
			std::size_t toAdd = data._positions.size();
			particleCountThisFrame += toAdd;

			// Because of the int cast when adding particles, which limits to 2*10^9 particles...
			if (particleCountThisFrame > std::numeric_limits<int>::max())
			{
				if (auto tmpIter = instance.addParticles(std::numeric_limits<int>::max());
					!firstAdd)
				{
					pIterator = tmpIter;
					firstAdd = true;
				}
				particleCountThisFrame -= std::numeric_limits<int>::max();
			}
		}

		auto tmpIter = instance.addParticles(static_cast<int>(particleCountThisFrame));
		if (!firstAdd)
		{
			pIterator = tmpIter;
		}

		return pIterator;
	}
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
	auto pIterator = resizeForThisFrame(*_particleInstance, frame);

	Partio::ParticleAccessor positionAccessor{ _blackboard->_position };
	pIterator.addAccessor(positionAccessor);

	Partio::ParticleAccessor idAccessor{ _blackboard->_id };
	pIterator.addAccessor(idAccessor);

	Partio::ParticleAccessor velocityAccessor{ _blackboard->_velocity };
	pIterator.addAccessor(velocityAccessor);

	/*Partio::ParticleAccessor timeAccessor{ _blackboard->_time };
	pIterator.addAccessor(timeAccessor);*/

	int uniformId = 0;

	for (const auto &data : frame._particleSystemElements)
	{
		if (this->shouldWriteData(data))
		{
			const std::vector<Storm::Vector3> &pPositions = data._positions;
			const std::vector<Storm::Vector3> &pVelocity = data._velocities;
			const std::size_t particleCount = pPositions.size();

			for (std::size_t iter = 0; iter < particleCount; ++iter)
			{
				//*timeAccessor.raw<float>(pIterator) = frame._physicsTime;
				memcpy(positionAccessor.raw<float>(pIterator), &pPositions[iter], sizeof(Storm::Vector3));
				*idAccessor.raw<int>(pIterator) = uniformId;
				memcpy(positionAccessor.raw<float>(pIterator), &pVelocity[iter], sizeof(Storm::Vector3));

				++uniformId;
				++pIterator;
			}
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

