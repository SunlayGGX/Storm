#include "RigidBody.h"

#include "PoissonDiskSampler.h"
#include "SceneRigidBodyConfig.h"
#include "SceneSimulationConfig.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"
#include "IGraphicsManager.h"
#include "IPhysicsManager.h"
#include "ISimulatorManager.h"

#include "AssetLoaderManager.h"

#include "MemoryHelper.h"
#include "Version.h"

#include "AssetCacheData.h"
#include "AssetCacheDataOrder.h"

#include <Assimp\Importer.hpp>
#include <Assimp\scene.h>

#include <boost\algorithm\string.hpp>

#include <fstream>

namespace
{
	std::filesystem::path computeRightCachedFilePath(const Storm::SceneRigidBodyConfig &rbSceneConfig, const std::filesystem::path &meshPath, float particleRadius)
	{
		std::string suffix;
		suffix.reserve(40);
		suffix += "_x";
		suffix += std::to_string(rbSceneConfig._scale.x());
		suffix += "_y";
		suffix += std::to_string(rbSceneConfig._scale.y());
		suffix += "_z";
		suffix += std::to_string(rbSceneConfig._scale.z());
		suffix += "_rad";
		suffix += std::to_string(particleRadius);
		suffix += "_l";
		suffix += Storm::toStdString(rbSceneConfig._layerGenerationMode);
		suffix += "x";
		suffix += std::to_string(rbSceneConfig._layerCount);

		boost::algorithm::replace_all(suffix, ".", "_");

		const std::filesystem::path meshFileName = std::filesystem::path{ meshPath.stem().string() + suffix }.replace_extension(".cPartRigidBody");

		return Storm::RigidBody::retrieveParticleDataCacheFolder() / meshFileName;
	}

	float computeLayerDistance(const float particleRadius)
	{
		return particleRadius * 2.f;
	}
}


Storm::RigidBody::RigidBody(const Storm::SceneRigidBodyConfig &rbSceneConfig) :
	_meshPath{ rbSceneConfig._meshFilePath },
	_rbId{ rbSceneConfig._rigidBodyID }
{
	this->load(rbSceneConfig);
}

Storm::RigidBody::RigidBody(const Storm::SceneRigidBodyConfig &rbSceneConfig, Storm::SystemSimulationStateObject &&state) :
	_meshPath{ rbSceneConfig._meshFilePath },
	_rbId{ rbSceneConfig._rigidBodyID }
{
	this->loadFromState(rbSceneConfig, std::move(state));
}

Storm::RigidBody::RigidBody(const Storm::SceneRigidBodyConfig &rbSceneConfig, ReplayMode) :
	_meshPath{ rbSceneConfig._meshFilePath },
	_rbId{ rbSceneConfig._rigidBodyID }
{
	this->loadForReplay(rbSceneConfig);
}

const std::string& Storm::RigidBody::getRigidBodyName() const
{
	return _meshPath;
}

unsigned int Storm::RigidBody::getRigidBodyID() const
{
	return _rbId;
}

void Storm::RigidBody::getRigidBodyTransform(Storm::Vector3 &outTrans, Storm::Vector3 &outRot) const
{
	// Physics module should have the ownership of this data.
	const Storm::IPhysicsManager &physicsMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IPhysicsManager>();
	physicsMgr.getMeshTransform(_rbId, outTrans, outRot);
}

void Storm::RigidBody::getRigidBodyTransform(Storm::Vector3 &outTrans, Storm::Quaternion &outRot) const
{
	// Physics module should have the ownership of this data.
	const Storm::IPhysicsManager &physicsMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IPhysicsManager>();
	physicsMgr.getMeshTransform(_rbId, outTrans, outRot);
}

std::vector<Storm::Vector3> Storm::RigidBody::getRigidBodyParticlesWorldPositions() const
{
	// Simulator module should have the ownership of this data.
	const Storm::ISimulatorManager &simulMgr = Storm::SingletonHolder::instance().getSingleton<Storm::ISimulatorManager>();
	return simulMgr.getParticleSystemPositions(_rbId);
}

std::vector<Storm::Vector3> Storm::RigidBody::getRigidBodyObjectSpaceVertexes() const
{
	// TODO : Graphics module should have the ownership of this data.
	throw std::logic_error("The method or operation is not implemented.");
}

std::vector<Storm::Vector3> Storm::RigidBody::getRigidBodyObjectSpaceNormals() const
{
	// TODO : Graphics module should have the ownership of this data.
	throw std::logic_error("The method or operation is not implemented.");
}

std::filesystem::path Storm::RigidBody::retrieveParticleDataCacheFolder()
{
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	return std::filesystem::path{ configMgr.getTemporaryPath() } / "ParticleData";
}

std::shared_ptr<Storm::AssetCacheData> Storm::RigidBody::baseLoadAssimp(const Storm::SceneRigidBodyConfig &rbSceneConfig, const float layerDist)
{
	std::shared_ptr<Storm::AssetCacheData> cachedDataPtr;

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

	Storm::AssetLoaderManager &assetLoaderMgr = Storm::AssetLoaderManager::instance();

	const auto pushDataToModules =
		[
			this,
			&graphicsMgr = singletonHolder.getSingleton<Storm::IGraphicsManager>(),
			&physicsMgr = singletonHolder.getSingleton<Storm::IPhysicsManager>(),
			&rbSceneConfig, 
			&assetLoaderMgr
		](const std::shared_ptr<Storm::AssetCacheData> &cachedDataPtr)
	{
		const std::vector<Storm::Vector3> &verticesPos = cachedDataPtr->getScaledVertices();
		const std::vector<Storm::Vector3> &normalsPos = cachedDataPtr->getScaledNormals();
		const std::vector<uint32_t> &indexes = cachedDataPtr->getIndices();

		std::lock_guard<std::mutex> lock{ assetLoaderMgr.getAddingMutex() };
		graphicsMgr.addMesh(_rbId, verticesPos, normalsPos, indexes);
		physicsMgr.addPhysicalBody(rbSceneConfig, verticesPos, indexes);
	};

	Storm::AssetCacheDataOrder order{ rbSceneConfig, nullptr, layerDist };
	order._considerFinalInEquivalence = true;

	cachedDataPtr = assetLoaderMgr.retrieveAssetData(order);
	if (cachedDataPtr == nullptr)
	{
		// The mesh is a separate entity, therefore we will load it with Assimp.
		Assimp::Importer meshImporter;
		order._assimpScene = meshImporter.ReadFile(_meshPath, 0);

		if (order._assimpScene->HasMeshes())
		{
			if (order._assimpScene->HasAnimations())
			{
				LOG_WARNING << "'" << _meshPath << "' contains animation. Animations aren't supported so they won't be handled!";
			}
			if (order._assimpScene->HasCameras())
			{
				LOG_WARNING << "'" << _meshPath << "' contains cameras. Embedded cameras aren't supported so they won't be handled!";
			}
			if (order._assimpScene->HasLights())
			{
				LOG_WARNING << "'" << _meshPath << "' contains lights. Lights aren't supported so they won't be handled!";
			}
			if (order._assimpScene->HasMaterials())
			{
				LOG_WARNING << "'" << _meshPath << "' contains materials. Materials aren't supported so they won't be handled!";
			}
			if (order._assimpScene->HasTextures())
			{
				LOG_WARNING << "'" << _meshPath << "' contains textures. Textures aren't supported so they won't be handled!";
			}

			cachedDataPtr = assetLoaderMgr.retrieveAssetData(order);
			pushDataToModules(cachedDataPtr);
		}
		else
		{
			Storm::throwException<Storm::Exception>("The mesh '" + _meshPath + "' doesn't have a mesh inside (an empty mesh). This isn't allowed!");
		}
	}
	else
	{
		pushDataToModules(cachedDataPtr);
	}

	return cachedDataPtr;
}

void Storm::RigidBody::load(const Storm::SceneRigidBodyConfig &rbSceneConfig)
{
	enum : uint64_t
	{
		// Those aren't really checksums but they'll do the same jobs and does it okay (no need to be extra secure, this is not confidential or running stuffs), so I would call them checksum.
		k_cachePlaceholderChecksum = 0x00AA00BB,
		k_cacheGoodChecksum = 0xABCDEF71,
	};

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
	const float currentParticleRadius = configMgr.getSceneSimulationConfig()._particleRadius;

	std::shared_ptr<Storm::AssetCacheData> cachedDataPtr = this->baseLoadAssimp(rbSceneConfig, computeLayerDistance(currentParticleRadius));

	std::vector<Storm::Vector3> particlePos;

	const std::string meshPathLowerStr = boost::algorithm::to_lower_copy(_meshPath);
	const std::filesystem::path meshPath = meshPathLowerStr;
	const std::filesystem::path cachedPath = computeRightCachedFilePath(rbSceneConfig, meshPath, currentParticleRadius);
	const std::wstring cachedPathStr = cachedPath.wstring();
	constexpr const Storm::Version currentVersion = Storm::Version::retrieveCurrentStormVersion();

	const auto srcMeshWriteTime = std::filesystem::last_write_time(meshPath);
	bool hasCache;

	bool mutexExisted;

	Storm::AssetLoaderManager &assetLoaderMgr = Storm::AssetLoaderManager::instance();
	std::mutex &specificMutex = assetLoaderMgr.getAssetMutex(cachedPath.filename().string(), mutexExisted);

	std::unique_lock<std::mutex> lock{ specificMutex };

	if (!mutexExisted && configMgr.shouldRegenerateParticleCache())
	{
		hasCache = false;
		LOG_COMMENT << "Regenerating cache no matter its state as requested by user from command line argument!";
		std::filesystem::remove_all(cachedPath);
	}
	else
	{
		hasCache = std::filesystem::exists(cachedPath);
	}

	if (hasCache)
	{
		std::ifstream cacheReadStream{ cachedPathStr, std::ios_base::in | std::ios_base::binary };

		uint64_t checksum;
		Storm::binaryRead(cacheReadStream, checksum);
		if (checksum == k_cacheGoodChecksum)
		{
			int64_t timestamp;
			Storm::binaryRead(cacheReadStream, timestamp);

			if (std::filesystem::file_time_type{ std::filesystem::file_time_type::duration{ timestamp } } == srcMeshWriteTime)
			{
				std::string srcUsedFilePath;
				Storm::binaryRead(cacheReadStream, srcUsedFilePath);

				// I used meshPathLowerStr instead of _meshPath because _meshPath was set by the user on the scene file, therefore comparison depends on the case of each letters.
				// meshPathLowerStr was constructed by the lowercase of _meshPath and is used to write to the binary file so it is suitable to use as a comparison (we know what we're expecting).
				if (meshPathLowerStr == srcUsedFilePath)
				{
					std::string versionTmp;
					Storm::binaryRead(cacheReadStream, versionTmp);

					Storm::Version cacheFileVersion{ versionTmp };
					if (cacheFileVersion != currentVersion)
					{
						LOG_WARNING << "'" << meshPath << "' has a particle cached file but it is was made with a previous version of the application (no retro compatibility), therefore we will regenerate it anew.";
						hasCache = false;
					}
				}
				else
				{
					LOG_WARNING << "'" << meshPath << "' has no particle cached file since the one we found was generated for another file that has the same name, therefore we will regenerate it anew.";
					hasCache = false;
				}
			}
			else
			{
				LOG_WARNING << "'" << meshPath << "' has a particle cached file but it is outdated, therefore we will regenerate it anew.";
				hasCache = false;
			}
		}
		else
		{
			LOG_WARNING << "'" << meshPath << "' has a particle cached file but it is corrupted (invalid), therefore we will regenerate it anew.";
			hasCache = false;
		}

		if (hasCache)
		{
			LOG_COMMENT << "'" << meshPath << "' has a right matching particle cached data file, therefore we will load it instead.";
			
			uint64_t particleCount;
			Storm::binaryRead(cacheReadStream, particleCount);
			particlePos.reserve(particleCount);
			for (uint64_t iter = 0; iter < particleCount; ++iter)
			{
				Storm::Vector3 &currentVect = particlePos.emplace_back();

				Storm::binaryRead(cacheReadStream, currentVect.x());
				Storm::binaryRead(cacheReadStream, currentVect.y());
				Storm::binaryRead(cacheReadStream, currentVect.z());
			}
		}
		else
		{
			cacheReadStream.close();
			std::filesystem::remove_all(cachedPath);
		}
	}

	if(!hasCache)
	{
		/* Generate the rb as if no cache data */

		// Poisson Disk sampling
		particlePos = Storm::PoissonDiskSampler::process_v2(30, currentParticleRadius, cachedDataPtr->getScaledVertices(), cachedDataPtr->getFinalBoundingBoxMax(), cachedDataPtr->getFinalBoundingBoxMin());
		
		/* Cache writing */

		std::ofstream cacheFileStream{ cachedPathStr, std::ios_base::out | std::ios_base::binary };

		// First write wrong checksum as a placeholder
		Storm::binaryWrite(cacheFileStream, static_cast<uint64_t>(k_cachePlaceholderChecksum));

		Storm::binaryWrite(cacheFileStream, static_cast<uint64_t>(srcMeshWriteTime.time_since_epoch().count()));
		Storm::binaryWrite(cacheFileStream, meshPathLowerStr);
		Storm::binaryWrite(cacheFileStream, static_cast<std::string>(currentVersion));

		Storm::binaryWrite(cacheFileStream, static_cast<uint64_t>(particlePos.size()));
		for (const Storm::Vector3 &particlePos : particlePos)
		{
			Storm::binaryWrite(cacheFileStream, particlePos.x());
			Storm::binaryWrite(cacheFileStream, particlePos.y());
			Storm::binaryWrite(cacheFileStream, particlePos.z());
		}

		// Replace the placeholder checksum by the right one to finalize the writing
		cacheFileStream.seekp(0);
		Storm::binaryWrite(cacheFileStream, static_cast<uint64_t>(k_cacheGoodChecksum));
	}

	lock.unlock();

	Storm::ISimulatorManager &simulMgr = singletonHolder.getSingleton<Storm::ISimulatorManager>();
	
	std::lock_guard<std::mutex> addingLock{ assetLoaderMgr.getAddingMutex() };
	simulMgr.addRigidBodyParticleSystem(_rbId, std::move(particlePos));
}

void Storm::RigidBody::loadForReplay(const Storm::SceneRigidBodyConfig &rbSceneConfig)
{
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	if (!configMgr.isInReplayMode())
	{
		Storm::throwException<Storm::Exception>(__FUNCSIG__ " should only be used in replay mode!");
	}

	this->baseLoadAssimp(rbSceneConfig, computeLayerDistance(configMgr.getSceneSimulationConfig()._particleRadius));
}

void Storm::RigidBody::loadFromState(const Storm::SceneRigidBodyConfig &rbSceneConfig, Storm::SystemSimulationStateObject &&state)
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	Storm::ISimulatorManager &simulMgr = singletonHolder.getSingleton<Storm::ISimulatorManager>();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

	Storm::AssetLoaderManager &assetLoaderMgr = Storm::AssetLoaderManager::instance();

	this->baseLoadAssimp(rbSceneConfig, computeLayerDistance(configMgr.getSceneSimulationConfig()._particleRadius));

	std::lock_guard<std::mutex> addingLock{ assetLoaderMgr.getAddingMutex() };
	simulMgr.addRigidBodyParticleSystem(std::move(state));
}
