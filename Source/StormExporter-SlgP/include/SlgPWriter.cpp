#include "SlgPWriter.h"

#include "IExporterConfigManager.h"
#include "SingletonHolder.h"

#include "Vector3.h"

#include "SerializeParticleSystemLayout.h"
#include "SerializeRecordHeader.h"
#include "SerializeRecordParticleSystemData.h"
#include "SerializeRecordPendingData.h"

#include "ExportMode.h"

#include "SerializePackage.h"
#include "SerializePackageCreationModality.h"

namespace
{
	struct ParticleData
	{
	public:
		ParticleData(uint32_t id, const Storm::Vector3 &position) :
			_id{ id },
			_position{ position }
		{}

	public:
		uint32_t _id;
		Storm::Vector3 _position;
	};

	enum : uint32_t
	{
		k_badMagicWord = 0,
		k_goodMagicWord = 0xFFAABB77
	};
}

namespace SlgPWriterPImplDetails
{
	struct SlgPDataWriterBlackboard
	{
	public:
		std::size_t _particleCount;

		std::list<std::pair<float, std::vector<ParticleData>>> _data;
	};
}


StormExporter::SlgPWriter::SlgPWriter(const Storm::SerializeRecordHeader &header) :
	_blackboard{ std::make_unique<std::remove_cvref_t<decltype(*_blackboard)>>() }
{
	LOG_COMMENT << "Record header parsed. We'll start writing to SlgP.";

	const auto &configMgr = Storm::SingletonHolder::instance().getSingleton<StormExporter::IExporterConfigManager>();
	const StormExporter::ExportMode mode = configMgr.getExportMode();

	const bool targetFluids = STORM_IS_BIT_ENABLED(mode, StormExporter::ExportMode::Fluid);
	const bool targetRigidBodies = STORM_IS_BIT_ENABLED(mode, StormExporter::ExportMode::RigidBody);

	std::size_t &particleCount = _blackboard->_particleCount;

	for (const auto &layout : header._particleSystemLayouts)
	{
		if (layout._particlesCount > 0 && 
			(
				(targetFluids && layout._isFluid) ||
				(targetRigidBodies && !layout._isFluid && !layout._isStatic)
			))
		{
			_targetIds.emplace_back(layout._particleSystemId);
			particleCount = layout._particlesCount;

			// We accept only one particle system for now.
			break;
		}
	}

	if (_targetIds.empty())
	{
		Storm::throwException<Storm::Exception>("No valid particle system exists inside the record file!");
	}
	if (particleCount == 0)
	{
		Storm::throwException<Storm::Exception>("Empty particle system!");
	}
}

StormExporter::SlgPWriter::~SlgPWriter() = default;

bool StormExporter::SlgPWriter::onFrameExport(const Storm::SerializeRecordPendingData &frame)
{
	if (const auto &exporterMgr = Storm::SingletonHolder::instance().getSingleton<StormExporter::IExporterConfigManager>();
		_blackboard->_data.size() > exporterMgr.getSliceOutFrames())
	{
		return false;
	}

	auto &thisFrameData = _blackboard->_data.emplace_back();

	thisFrameData.first = frame._physicsTime;
	thisFrameData.second.reserve(_blackboard->_particleCount);

	for (const auto &data : frame._particleSystemElements)
	{
		if (this->shouldWriteData(data))
		{
			const std::vector<Storm::Vector3> &pPositions = data._positions;
			const std::size_t particleCount = pPositions.size();

			for (std::size_t iter = 0; iter < particleCount; ++iter)
			{
				thisFrameData.second.emplace_back(static_cast<uint32_t>(iter), pPositions[iter]);
			}
		}
	}

	return true;
}

void StormExporter::SlgPWriter::onExportClose()
{
	const auto &configMgr = Storm::SingletonHolder::instance().getSingleton<StormExporter::IExporterConfigManager>();
	const std::string &fileToExport = configMgr.getOutExportPath();

	std::filesystem::create_directories(std::filesystem::path{ fileToExport }.parent_path());

	LOG_COMMENT << "Finished transferring data to slgP.\nWriting slgP file at '" << fileToExport << '\'';

	Storm::SerializePackage package{ Storm::SerializePackageCreationModality::SavingNewPreheaderProvidedAfter, fileToExport };

	uint32_t magicWord = k_badMagicWord;
	package << magicWord;

	float currentVersion = 1.0;
	package << currentVersion;

	uint64_t frameCount = _blackboard->_data.size();
	package << frameCount;

	uint64_t pCount = _blackboard->_particleCount;
	package << pCount;

	for (auto &dataThisFrame : _blackboard->_data)
	{
		package << dataThisFrame.first;

		for (ParticleData &pData : dataThisFrame.second)
		{
			package <<
				pData._id <<
				pData._position;
		}
	}

	package.seekAbsolute(0);
	magicWord = k_goodMagicWord;
	package << magicWord;

	package.flush();
}

bool StormExporter::SlgPWriter::shouldWriteData(const Storm::SerializeRecordParticleSystemData &pSystemData) const
{
	return std::any_of(std::begin(_targetIds), std::end(_targetIds), [id = pSystemData._systemId](const unsigned int targetId)
	{
		return targetId == id;
	});
}

