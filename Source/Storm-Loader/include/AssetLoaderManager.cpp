#include "AssetLoaderManager.h"

#include "AssimpLoggingWrapper.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"
#include "IGraphicsManager.h"
#include "IPhysicsManager.h"
#include "ISimulatorManager.h"
#include "IThreadManager.h"

#include "GeneralSimulationData.h"
#include "FluidData.h"
#include "BlowerData.h"
#include "RigidBodySceneData.h"

#include "RigidBody.h"

#include "BasicMeshGenerator.h"

#include "FluidParticleLoadDenseMode.h"

#include "BlowerMeshMaker.h"
#include "BlowerType.h"

#include "ThreadEnumeration.h"

#include "AssetCacheData.h"
#include "AssetCacheDataOrder.h"

#include <Assimp\DefaultLogger.hpp>


namespace
{
	void initializeAssimpLogger()
	{
		Assimp::Logger* defaultLogger = Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
		if (!defaultLogger->attachStream(new Storm::AssimpDebugLoggerStream, Assimp::Logger::ErrorSeverity::Debugging))
		{
			LOG_WARNING << "Assimp Logger Debug stream failed to attach";
		}

		if (!defaultLogger->attachStream(new Storm::AssimpInfoLoggerStream, Assimp::Logger::ErrorSeverity::Info))
		{
			LOG_WARNING << "Assimp Logger Info stream failed to attach";
		}

		if (!defaultLogger->attachStream(new Storm::AssimpWarningLoggerStream, Assimp::Logger::ErrorSeverity::Warn))
		{
			LOG_WARNING << "Assimp Logger Warning stream failed to attach";
		}

		if (!defaultLogger->attachStream(new Storm::AssimpErrorLoggerStream, Assimp::Logger::ErrorSeverity::Err))
		{
			LOG_WARNING << "Assimp Logger Error stream failed to attach";
		}
	}

	template<Storm::FluidParticleLoadDenseMode>
	void computeParticleCountBoxExtents(const Storm::FluidBlockData &fluidBlock, const float particleDiameter, std::size_t &outParticleXCount, std::size_t &outParticleYCount, std::size_t &outParticleZCount);

	template<Storm::FluidParticleLoadDenseMode>
	void generateFluidParticles(std::vector<Storm::Vector3> &inOutFluidParticlePos, const Storm::FluidBlockData &fluidBlockGenerated, const float particleDiameter);


	template<>
	void computeParticleCountBoxExtents<Storm::FluidParticleLoadDenseMode::Normal>(const Storm::FluidBlockData &fluidBlock, const float particleDiameter, std::size_t &outParticleXCount, std::size_t &outParticleYCount, std::size_t &outParticleZCount)
	{
		outParticleXCount = static_cast<std::size_t>(fabs(fluidBlock._firstPoint.x() - fluidBlock._secondPoint.x()) / particleDiameter);
		outParticleYCount = static_cast<std::size_t>(fabs(fluidBlock._firstPoint.y() - fluidBlock._secondPoint.y()) / particleDiameter);
		outParticleZCount = static_cast<std::size_t>(fabs(fluidBlock._firstPoint.z() - fluidBlock._secondPoint.z()) / particleDiameter);
	}

	template<>
	void computeParticleCountBoxExtents<Storm::FluidParticleLoadDenseMode::AsSplishSplash>(const Storm::FluidBlockData &fluidBlock, const float particleDiameter, std::size_t &outParticleXCount, std::size_t &outParticleYCount, std::size_t &outParticleZCount)
	{
		const Storm::Vector3 diff = fluidBlock._secondPoint - fluidBlock._firstPoint;

		outParticleXCount = static_cast<std::size_t>(std::roundf(diff[0] / particleDiameter)) - 1ULL;
		outParticleYCount = static_cast<std::size_t>(std::roundf(diff[1] / particleDiameter)) - 1ULL;
		outParticleZCount = static_cast<std::size_t>(std::roundf(diff[2] / particleDiameter)) - 1ULL;
	}

	template<>
	void generateFluidParticles<Storm::FluidParticleLoadDenseMode::Normal>(std::vector<Storm::Vector3> &inOutFluidParticlePos, const Storm::FluidBlockData &fluidBlockGenerated, const float particleDiameter)
	{
		std::size_t particleXCount;
		std::size_t particleYCount;
		std::size_t particleZCount;

		computeParticleCountBoxExtents<Storm::FluidParticleLoadDenseMode::Normal>(fluidBlockGenerated, particleDiameter, particleXCount, particleYCount, particleZCount);

		const float xPosBegin = std::min(fluidBlockGenerated._firstPoint.x(), fluidBlockGenerated._secondPoint.x());
		const float yPosBegin = std::min(fluidBlockGenerated._firstPoint.y(), fluidBlockGenerated._secondPoint.y());
		const float zPosBegin = std::min(fluidBlockGenerated._firstPoint.z(), fluidBlockGenerated._secondPoint.z());

		Storm::Vector3 currentParticlePos;
		for (std::size_t xIter = 0; xIter < particleXCount; ++xIter)
		{
			currentParticlePos.x() = xPosBegin + static_cast<float>(xIter) * particleDiameter;
			for (std::size_t yIter = 0; yIter < particleYCount; ++yIter)
			{
				currentParticlePos.y() = yPosBegin + static_cast<float>(yIter) * particleDiameter;
				for (std::size_t zIter = 0; zIter < particleZCount; ++zIter)
				{
					currentParticlePos.z() = zPosBegin + static_cast<float>(zIter) * particleDiameter;
					inOutFluidParticlePos.push_back(currentParticlePos);
				}
			}
		}
	}

	template<>
	void generateFluidParticles<Storm::FluidParticleLoadDenseMode::AsSplishSplash>(std::vector<Storm::Vector3> &inOutFluidParticlePos, const Storm::FluidBlockData &fluidBlockGenerated, const float particleDiameter)
	{
		std::size_t particleXCount;
		std::size_t particleYCount;
		std::size_t particleZCount;

		computeParticleCountBoxExtents<Storm::FluidParticleLoadDenseMode::AsSplishSplash>(fluidBlockGenerated, particleDiameter, particleXCount, particleYCount, particleZCount);

		const Storm::Vector3 start = fluidBlockGenerated._firstPoint + particleDiameter * Storm::Vector3::Ones();

		for (std::size_t xIter = 0; xIter < particleXCount; ++xIter)
		{
			const float xCoeff = static_cast<float>(xIter);
			for (std::size_t yIter = 0; yIter < particleYCount; ++yIter)
			{
				const float yCoeff = static_cast<float>(yIter);
				for (std::size_t zIter = 0; zIter < particleZCount; ++zIter)
				{
					const float zCoeff = static_cast<float>(zIter);
					Storm::Vector3 &addedPos = inOutFluidParticlePos.emplace_back(xCoeff * particleDiameter, yCoeff * particleDiameter, zCoeff * particleDiameter);
					addedPos += start;
				}
			}
		}
	}


}

#define STORM_EXECUTE_CASE_ON_DENSE_MODE(DenseModeEnum) case Storm::##DenseModeEnum: STORM_EXECUTE_METHOD_ON_DENSE_MODE(Storm::##DenseModeEnum) break


Storm::AssetLoaderManager::AssetLoaderManager() = default;
Storm::AssetLoaderManager::~AssetLoaderManager() = default;

void Storm::AssetLoaderManager::initialize_Implementation()
{
	LOG_COMMENT << "Asset loading started! Depending on the caching state of each assets, it could take some time...";

	initializeAssimpLogger();

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	
	std::filesystem::create_directories(Storm::RigidBody::retrieveParticleDataCacheFolder());

	Storm::IGraphicsManager &graphicsMgr = singletonHolder.getSingleton<Storm::IGraphicsManager>();
	Storm::IPhysicsManager &physicsMgr = singletonHolder.getSingleton<Storm::IPhysicsManager>();
	Storm::ISimulatorManager &simulMgr = singletonHolder.getSingleton<Storm::ISimulatorManager>();
	Storm::IThreadManager &threadMgr = singletonHolder.getSingleton<Storm::IThreadManager>();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

	/* Load rigid bodies */
	LOG_COMMENT << "Loading Rigid body";

	const auto &rigidBodiesDataToLoad = configMgr.getRigidBodiesData();
	const std::size_t rigidBodyCount = rigidBodiesDataToLoad.size();
	_rigidBodies.reserve(rigidBodyCount);
	for (const auto &rbToLoad : rigidBodiesDataToLoad)
	{
		auto &emplacedRb = _rigidBodies.emplace_back(std::static_pointer_cast<Storm::IRigidBody>(std::make_shared<Storm::RigidBody>(rbToLoad)));
		const unsigned int emplacedRbId = emplacedRb->getRigidBodyID();

		graphicsMgr.bindParentRbToMesh(emplacedRbId, emplacedRb);
		physicsMgr.bindParentRbToPhysicalBody(rbToLoad, emplacedRb);

		LOG_DEBUG << "Rigid body " << emplacedRbId << " created and bound to the right modules.";
	}

	/* Load constraints */
	const auto &constraintsData = configMgr.getConstraintsData();
	if (!constraintsData.empty())
	{
		LOG_COMMENT << "Loading Constraints";
		physicsMgr.loadConstraints(constraintsData);
	}
	else
	{
		LOG_DEBUG << "No Constraints to load. Skipping this step.";
	}

	/* Loading Blowers */
	const auto &blowersDataToLoad = configMgr.getBlowersData();

	std::vector<Storm::Vector3> areaVertexesTmp;
	std::vector<uint32_t> areaIndexesTmp;
	for (const Storm::BlowerData &blowerToLoad : blowersDataToLoad)
	{
		// Generate the mesh area data to pass to the graphical resources.
#define STORM_XMACRO_GENERATE_ELEMENTARY_BLOWER(BlowerTypeName, BlowerTypeXmlName, EffectAreaType, MeshMakerType) \
case Storm::BlowerType::BlowerTypeName: \
	Storm::MeshMakerType::generate(blowerToLoad, areaVertexesTmp, areaIndexesTmp); \
	break;

		switch (blowerToLoad._blowerType)
		{
			STORM_XMACRO_GENERATE_BLOWERS_CODE;

		default:
		case Storm::BlowerType::None:
			Storm::throwException<std::exception>("Unknown Blower to be created!");
			break;
		}

#undef STORM_XMACRO_GENERATE_ELEMENTARY_BLOWER

		threadMgr.executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [&graphicsMgr, &blowerToLoad, areaVertexes = std::move(areaVertexesTmp), areaIndexes = std::move(areaIndexesTmp)]()
		{
			graphicsMgr.loadBlower(blowerToLoad, areaVertexes, areaIndexes);
		});

		simulMgr.loadBlower(blowerToLoad);
	}

	/* Load fluid particles */
	const Storm::GeneralSimulationData &generalConfigData = configMgr.getGeneralSimulationData();
	if (generalConfigData._hasFluid)
	{
		LOG_COMMENT << "Loading fluid particles";

		const float particleRadius = generalConfigData._particleRadius;
		const float particleDiameter = particleRadius * 2.f;
		const auto &fluidsDataToLoad = configMgr.getFluidData();
		std::vector<Storm::Vector3> fluidParticlePos;

		// First, evaluate the particle total count we need to have (to avoid unneeded reallocations along the way)...
		fluidParticlePos.reserve(std::accumulate(std::begin(fluidsDataToLoad._fluidGenData), std::end(fluidsDataToLoad._fluidGenData), static_cast<std::size_t>(0),
			[particleDiameter](const std::size_t accumulatedVal, const Storm::FluidBlockData &fluidBlockGenerated)
		{
			std::size_t particleXCount;
			std::size_t particleYCount;
			std::size_t particleZCount;

			switch (fluidBlockGenerated._loadDenseMode)
			{
#define STORM_EXECUTE_METHOD_ON_DENSE_MODE(DenseMode) computeParticleCountBoxExtents<DenseMode>(fluidBlockGenerated, particleDiameter, particleXCount, particleYCount, particleZCount);

				STORM_EXECUTE_CASE_ON_DENSE_MODE(FluidParticleLoadDenseMode::Normal);
				STORM_EXECUTE_CASE_ON_DENSE_MODE(FluidParticleLoadDenseMode::AsSplishSplash);

#undef STORM_EXECUTE_METHOD_ON_DENSE_MODE

			default:
				break;
			}

			return accumulatedVal + particleXCount * particleYCount * particleZCount;
		}));

		for (const Storm::FluidBlockData &fluidBlockGenerated : fluidsDataToLoad._fluidGenData)
		{
			switch (fluidBlockGenerated._loadDenseMode)
			{
#define STORM_EXECUTE_METHOD_ON_DENSE_MODE(DenseMode) generateFluidParticles<DenseMode>(fluidParticlePos, fluidBlockGenerated, particleDiameter);

				STORM_EXECUTE_CASE_ON_DENSE_MODE(FluidParticleLoadDenseMode::Normal);
				STORM_EXECUTE_CASE_ON_DENSE_MODE(FluidParticleLoadDenseMode::AsSplishSplash);

#undef STORM_EXECUTE_METHOD_ON_DENSE_MODE

			default:
				break;
			}
		}

		// We need to update the position to regenerate the position of any rigid body particle according to its translation.
		// This needs to be done only for rigid bodies. Fluids don't need it. 
		simulMgr.refreshParticlesPosition();

		simulMgr.addFluidParticleSystem(fluidsDataToLoad._fluidId, std::move(fluidParticlePos));
	}
	else
	{
		LOG_WARNING << 
			"No fluid was present and we allowed it... Therefore, we would skip fluid loading to concentrate on rigid body interactions.\n"
			"It is a debug/development helper feature so don't let this setting remains because this isn't the purpose of the application...";

		simulMgr.refreshParticlesPosition();
	}

	LOG_DEBUG << "Cleaning asset loading cache data";
	this->clearCachedAssetData();

	LOG_COMMENT << "Asset loading finished!";
}

void Storm::AssetLoaderManager::cleanUp_Implementation()
{
	Assimp::DefaultLogger::kill();
}

const std::vector<std::shared_ptr<Storm::IRigidBody>>& Storm::AssetLoaderManager::getRigidBodyArray() const
{
	return _rigidBodies;
}

void Storm::AssetLoaderManager::generateSimpleCube(const Storm::Vector3 &position, const Storm::Vector3 &dimension, std::vector<Storm::Vector3> &inOutVertexes, std::vector<uint32_t> &inOutIndexes) const
{
	Storm::BasicMeshGenerator::generateCube(position, dimension, inOutVertexes, inOutIndexes);
}

void Storm::AssetLoaderManager::generateSimpleSphere(const Storm::Vector3 &position, const float radius, std::vector<Storm::Vector3> &inOutVertexes, std::vector<uint32_t> &inOutIndexes) const
{
	Storm::BasicMeshGenerator::generateSphere(position, radius, inOutVertexes, inOutIndexes);
}

void Storm::AssetLoaderManager::generateSimpleCylinder(const Storm::Vector3 &position, const float radius, const float height, std::vector<Storm::Vector3> &inOutVertexes, std::vector<uint32_t> &inOutIndexes) const
{
	Storm::BasicMeshGenerator::generateCylinder(position, radius, height, inOutVertexes, inOutIndexes);
}

void Storm::AssetLoaderManager::generateSimpleCone(const Storm::Vector3 &position, const float upRadius, const float downRadius, const float height, std::vector<Storm::Vector3> &inOutVertexes, std::vector<uint32_t> &inOutIndexes) const
{
	Storm::BasicMeshGenerator::generateCone(position, upRadius, downRadius, height, inOutVertexes, inOutIndexes);
}

std::shared_ptr<Storm::AssetCacheData> Storm::AssetLoaderManager::retrieveAssetData(const Storm::AssetCacheDataOrder &order)
{
	auto &assetCacheDataArray = _cachedAssetData[order._rbConfig._meshFilePath];
	if (!assetCacheDataArray.empty())
	{
		for (const std::shared_ptr<Storm::AssetCacheData> &assetCacheDataPtr : assetCacheDataArray)
		{
			if (assetCacheDataPtr->isEquivalentWith(order._rbConfig, order._considerFinalInEquivalence))
			{
				return assetCacheDataPtr;
			}
		}

		// We already created a source so we can skip the mesh reimporting.
		return assetCacheDataArray.emplace_back(std::make_shared<Storm::AssetCacheData>(order._rbConfig, *assetCacheDataArray.front()));
	}

	if (order._assimpScene)
	{
		return assetCacheDataArray.emplace_back(std::make_shared<Storm::AssetCacheData>(order._rbConfig, order._assimpScene));
	}
	else
	{
		return nullptr;
	}
}

void Storm::AssetLoaderManager::clearCachedAssetData()
{
	_cachedAssetData.clear();
}
