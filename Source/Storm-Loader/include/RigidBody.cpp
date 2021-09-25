#include "RigidBody.h"

#include "PoissonDiskSampler.h"
#include "UniformSampler.h"

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

#include "CollisionType.h"
#include "VolumeComputationTechnique.h"

#include "AssetVolumeIntegrator.h"
#include "LayeringGenerationTechnique.h"
#include "GeometryType.h"

#include "RunnerHelper.h"

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
		if (rbSceneConfig._layerGenerationMode == Storm::LayeringGenerationTechnique::Uniform)
		{
			suffix += "_type";
			suffix += Storm::toStdString(rbSceneConfig._geometry);

			if (rbSceneConfig._geometry == Storm::GeometryType::EquiSphere_MarkusDeserno)
			{
				suffix += "x";
				suffix += Storm::toStdString(rbSceneConfig._sampleCountMDeserno);
			}
		}

		suffix += "x";
		suffix += std::to_string(rbSceneConfig._layerCount);

		if (rbSceneConfig._isWall)
		{
			suffix += "ext";
		}
		else
		{
			suffix += "int";
		}

		suffix += "_nc";
		suffix += Storm::toStdString<Storm::NumericPolicy>(rbSceneConfig._enforceNormalsCoherency);

		boost::algorithm::replace_all(suffix, ".", "_");

		const std::filesystem::path meshFileName = std::filesystem::path{ meshPath.stem().string() + suffix }.replace_extension(".cPartRigidBody");

		return Storm::RigidBody::retrieveParticleDataCacheFolder() / meshFileName;
	}

	float computeLayerDistance(const float particleRadius)
	{
		return particleRadius * 2.f;
	}

	// We make the assumption the center of the rigid body bound to the passed normals is the origin of the domain { 0, 0, 0 }.
	// Note : this method purpose is to fix incoherency from good data. For example, to fix normals that points to the wrong way because a triangle normal was computed clockwise while another was computed counterclockwise.
	// But we cannot correct a mesh those triangles are in a mess.
	template<bool toTheOutside>
	void enforceNormalsCoherency(Storm::SamplingResult &inOutSamplingResult)
	{
		const std::vector<Storm::Vector3> &positions = inOutSamplingResult._position;
		std::vector<Storm::Vector3> &normals = inOutSamplingResult._normals;

		const std::size_t normalsCount = normals.size();
		for (std::size_t iter = 0; iter < normalsCount; ++iter)
		{
			const Storm::Vector3 &currentPPosition = positions[iter];
			Storm::Vector3 &currentPNormal = normals[iter];
			const float scalar = currentPPosition.dot(currentPNormal);

			bool normalWrongWay;
			if constexpr (toTheOutside)
			{
				normalWrongWay = scalar < 0.f;
			}
			else
			{
				normalWrongWay = scalar > 0.f;
			}

			if (normalWrongWay)
			{
				currentPNormal = -currentPNormal;
			}
		}
	}
}


Storm::RigidBody::RigidBody(const Storm::SceneRigidBodyConfig &rbSceneConfig) :
	_meshPath{ rbSceneConfig._meshFilePath },
	_rbId{ rbSceneConfig._rigidBodyID },
	_rbVolume{ -1.f }
{
	this->load(rbSceneConfig);
}

Storm::RigidBody::RigidBody(const Storm::SceneRigidBodyConfig &rbSceneConfig, Storm::SystemSimulationStateObject &&state) :
	_meshPath{ rbSceneConfig._meshFilePath },
	_rbId{ rbSceneConfig._rigidBodyID },
	_rbVolume{ -1.f }
{
	this->loadFromState(rbSceneConfig, std::move(state));
}

Storm::RigidBody::RigidBody(const Storm::SceneRigidBodyConfig &rbSceneConfig, ReplayMode) :
	_meshPath{ rbSceneConfig._meshFilePath },
	_rbId{ rbSceneConfig._rigidBodyID },
	_rbVolume{ -1.f }
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

void Storm::RigidBody::getRigidBodyTransform(Storm::Vector3 &outTrans, Storm::Rotation &outRot) const
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

float Storm::RigidBody::getRigidBodyVolume() const
{
	return _rbVolume;
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

	std::shared_ptr<Storm::AssetCacheData> cachedDataPtr;

	bool isAloneParticle;
	switch (rbSceneConfig._collisionShape)
	{
	case Storm::CollisionType::IndividualParticle:
	{
		Storm::SceneRigidBodyConfig tmpRbConfig = rbSceneConfig;
		tmpRbConfig._layerCount = 1;

		cachedDataPtr = this->baseLoadAssimp(tmpRbConfig, currentParticleRadius);

		isAloneParticle = true;
		break;
	}

	case Storm::CollisionType::Cube:
	case Storm::CollisionType::Custom:
	case Storm::CollisionType::Sphere:
	case Storm::CollisionType::None:
	default:
		cachedDataPtr = this->baseLoadAssimp(rbSceneConfig, computeLayerDistance(currentParticleRadius));
		isAloneParticle = false;
		break;
	}

	Storm::SamplingResult samplingResult;
	Storm::AssetLoaderManager &assetLoaderMgr = Storm::AssetLoaderManager::instance();

	if (!isAloneParticle)
	{
		const std::string meshPathLowerStr = boost::algorithm::to_lower_copy(_meshPath);
		const std::filesystem::path meshPath = meshPathLowerStr;
		const std::filesystem::path cachedPath = computeRightCachedFilePath(rbSceneConfig, meshPath, currentParticleRadius);
		const std::wstring cachedPathStr = cachedPath.wstring();
		constexpr const Storm::Version currentVersion = Storm::Version::retrieveCurrentStormVersion();

		const auto srcMeshWriteTime = std::filesystem::last_write_time(meshPath);
		bool hasCache;

		bool mutexExisted;

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

						try
						{
							Storm::Version cacheFileVersion{ versionTmp };
							if (cacheFileVersion != currentVersion)
							{
								LOG_WARNING << "'" << meshPath << "' has a particle cached file but it is was made with a previous version of the application (no retro compatibility), therefore we will regenerate it anew.";
								hasCache = false;
							}
						}
						catch (const Storm::Exception &e)
						{
							LOG_ERROR <<
								"Cache version parsing failed. We'll invalidate the cache. Reason was " << e.what() << "\n\n"
								"Stack trace :\n" <<
								e.stackTrace();

							hasCache = false;
						}
						catch (const std::exception &e)
						{
							LOG_ERROR << "Cache version parsing failed. We'll invalidate the cache. Reason was " << e.what();
							hasCache = false;
						}
						catch (...)
						{
							LOG_ERROR << "Cache version parsing failed for unknown reasons. We'll invalidate the cache";
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

				samplingResult._position.reserve(particleCount);
				for (uint64_t iter = 0; iter < particleCount; ++iter)
				{
					Storm::Vector3 &currentPos = samplingResult._position.emplace_back();
					Storm::binaryRead(cacheReadStream, currentPos.x());
					Storm::binaryRead(cacheReadStream, currentPos.y());
					Storm::binaryRead(cacheReadStream, currentPos.z());
				}

				samplingResult._normals.reserve(particleCount);
				for (uint64_t iter = 0; iter < particleCount; ++iter)
				{
					Storm::Vector3 &currentNormal = samplingResult._normals.emplace_back();
					Storm::binaryRead(cacheReadStream, currentNormal.x());
					Storm::binaryRead(cacheReadStream, currentNormal.y());
					Storm::binaryRead(cacheReadStream, currentNormal.z());
				}
			}
			else
			{
				cacheReadStream.close();
				std::filesystem::remove_all(cachedPath);
			}
		}

		if (!hasCache)
		{
			/* Generate the rb as if no cache data */

			// Poisson Disk sampling
			if (rbSceneConfig._layerGenerationMode == Storm::LayeringGenerationTechnique::Uniform)
			{
				const float sepDistance = currentParticleRadius * 2.f;
				const bool internalLayer = !rbSceneConfig._isWall;
				switch (rbSceneConfig._geometry)
				{
				case Storm::GeometryType::Cube:
					if (internalLayer)
					{
						samplingResult = Storm::UniformSampler::process<true>(rbSceneConfig._geometry, sepDistance, rbSceneConfig._layerCount, &rbSceneConfig._scale);
					}
					else
					{
						samplingResult = Storm::UniformSampler::process<false>(rbSceneConfig._geometry, sepDistance, rbSceneConfig._layerCount, &rbSceneConfig._scale);
					}
					break;

				case Storm::GeometryType::Sphere:
					if (internalLayer)
					{
						samplingResult = Storm::UniformSampler::process<true>(rbSceneConfig._geometry, sepDistance, rbSceneConfig._layerCount, &rbSceneConfig._scale.x());
					}
					else
					{
						samplingResult = Storm::UniformSampler::process<false>(rbSceneConfig._geometry, sepDistance, rbSceneConfig._layerCount, &rbSceneConfig._scale.x());
					}
					break;

				case Storm::GeometryType::EquiSphere_MarkusDeserno:
				{
					std::pair<float, std::size_t> data{ rbSceneConfig._scale.x(), rbSceneConfig._sampleCountMDeserno };
					if (internalLayer)
					{
						samplingResult = Storm::UniformSampler::process<true>(rbSceneConfig._geometry, sepDistance, rbSceneConfig._layerCount, &data);
					}
					else
					{
						samplingResult = Storm::UniformSampler::process<false>(rbSceneConfig._geometry, sepDistance, rbSceneConfig._layerCount, &data);
					}
					break;
				}

				default:
					Storm::throwException<Storm::Exception>("Unhandled Geometry Type : " + Storm::toStdString(rbSceneConfig._geometry));
				}
			}
			else
			{
				switch (rbSceneConfig._collisionShape)
				{
				case Storm::CollisionType::Sphere:
					samplingResult = Storm::PoissonDiskSampler::process_v2(30, currentParticleRadius, cachedDataPtr->getScaledVertices(), cachedDataPtr->getFinalBoundingBoxMax(), cachedDataPtr->getFinalBoundingBoxMin(), true);
					// Smooth them
					Storm::runParallel(samplingResult._position, [&samplingResult](const Storm::Vector3 &currentPPosition, const std::size_t currentPIndex)
					{
						// The center is { 0, 0, 0 } in object coordinate
						samplingResult._normals[currentPIndex] = currentPPosition.normalized();
					});
					break;

				case CollisionType::None:
				case CollisionType::Cube:
				case CollisionType::IndividualParticle:
				case CollisionType::Custom:
				default:
					samplingResult = Storm::PoissonDiskSampler::process_v2(30, currentParticleRadius, cachedDataPtr->getScaledVertices(), cachedDataPtr->getFinalBoundingBoxMax(), cachedDataPtr->getFinalBoundingBoxMin(), false);
					break;
				}
			}

			if (samplingResult._position.size() != samplingResult._normals.size())
			{
				Storm::throwException<Storm::Exception>("Mismatch between normals and positions. Something went wrong while sampling the rigidbody " + std::to_string(_rbId) + " !");
			}

			if (rbSceneConfig._enforceNormalsCoherency)
			{
				if (rbSceneConfig._isWall)
				{
					enforceNormalsCoherency<false>(samplingResult);
				}
				else
				{
					enforceNormalsCoherency<true>(samplingResult);
				}
			}

			/* Cache writing */

			std::ofstream cacheFileStream{ cachedPathStr, std::ios_base::out | std::ios_base::binary };

			// First write wrong checksum as a placeholder
			Storm::binaryWrite(cacheFileStream, static_cast<uint64_t>(k_cachePlaceholderChecksum));

			Storm::binaryWrite(cacheFileStream, static_cast<uint64_t>(srcMeshWriteTime.time_since_epoch().count()));
			Storm::binaryWrite(cacheFileStream, meshPathLowerStr);
			Storm::binaryWrite(cacheFileStream, static_cast<std::string>(currentVersion));

			Storm::binaryWrite(cacheFileStream, static_cast<uint64_t>(samplingResult._position.size()));
			for (const Storm::Vector3 &particlePos : samplingResult._position)
			{
				Storm::binaryWrite(cacheFileStream, particlePos.x());
				Storm::binaryWrite(cacheFileStream, particlePos.y());
				Storm::binaryWrite(cacheFileStream, particlePos.z());
			}
			
			for (const Storm::Vector3 &particleNormal : samplingResult._normals)
			{
				Storm::binaryWrite(cacheFileStream, particleNormal.x());
				Storm::binaryWrite(cacheFileStream, particleNormal.y());
				Storm::binaryWrite(cacheFileStream, particleNormal.z());
			}

			// Replace the placeholder checksum by the right one to finalize the writing
			cacheFileStream.seekp(0);
			Storm::binaryWrite(cacheFileStream, static_cast<uint64_t>(k_cacheGoodChecksum));
		}

		lock.unlock();

		this->initializeVolume(rbSceneConfig, cachedDataPtr.get());
	}
	else
	{
		samplingResult._position.emplace_back(Storm::Vector3::Zero());
		samplingResult._normals.emplace_back(Storm::Vector3::Zero());
	}

	Storm::ISimulatorManager &simulMgr = singletonHolder.getSingleton<Storm::ISimulatorManager>();
	
	std::lock_guard<std::mutex> addingLock{ assetLoaderMgr.getAddingMutex() };
	simulMgr.addRigidBodyParticleSystem(_rbId, std::move(samplingResult._position), std::move(samplingResult._normals));
}

void Storm::RigidBody::loadForReplay(const Storm::SceneRigidBodyConfig &rbSceneConfig)
{
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	if (!configMgr.isInReplayMode())
	{
		Storm::throwException<Storm::Exception>(__FUNCSIG__ " should only be used in replay mode!");
	}

	const std::shared_ptr<Storm::AssetCacheData> cachedDataPtr = this->baseLoadAssimp(rbSceneConfig, computeLayerDistance(configMgr.getSceneSimulationConfig()._particleRadius));
	this->initializeVolume(rbSceneConfig, cachedDataPtr.get());
}

void Storm::RigidBody::loadFromState(const Storm::SceneRigidBodyConfig &rbSceneConfig, Storm::SystemSimulationStateObject &&state)
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	Storm::ISimulatorManager &simulMgr = singletonHolder.getSingleton<Storm::ISimulatorManager>();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

	Storm::AssetLoaderManager &assetLoaderMgr = Storm::AssetLoaderManager::instance();

	const std::shared_ptr<Storm::AssetCacheData> cachedDataPtr = this->baseLoadAssimp(rbSceneConfig, computeLayerDistance(configMgr.getSceneSimulationConfig()._particleRadius));
	this->initializeVolume(rbSceneConfig, cachedDataPtr.get());

	std::lock_guard<std::mutex> addingLock{ assetLoaderMgr.getAddingMutex() };
	simulMgr.addRigidBodyParticleSystem(std::move(state));
}

void Storm::RigidBody::initializeVolume(const Storm::SceneRigidBodyConfig &rbSceneConfig, const Storm::AssetCacheData*const assetCachedData)
{
	bool useMeshCompute;

	switch (rbSceneConfig._volumeComputationTechnique)
	{
	case Storm::VolumeComputationTechnique::None:
		return;

	case Storm::VolumeComputationTechnique::Auto:
		useMeshCompute = rbSceneConfig._collisionShape == Storm::CollisionType::Custom;
		break;

	case Storm::VolumeComputationTechnique::TriangleIntegration:
		useMeshCompute = true;
		break;

	default:
		Storm::throwException<Storm::Exception>("Unknown volume computation technique (" + Storm::toStdString(rbSceneConfig._volumeComputationTechnique) + ")!");
	}

	if (useMeshCompute)
	{
		if (assetCachedData == nullptr)
		{
			LOG_DEBUG_ERROR << "Asset cache data is null. Cannot compute the rigid body volume!";
			return;
		}

		switch (rbSceneConfig._volumeComputationTechnique)
		{
		case Storm::VolumeComputationTechnique::TriangleIntegration:
			_rbVolume = Storm::AssetVolumeIntegrator::computeTriangleMeshVolume(*assetCachedData);
			break;

		case Storm::VolumeComputationTechnique::Auto:
		case Storm::VolumeComputationTechnique::None:
		default:
			assert(false && "We should have handled the technique to use decision over mesh compute in the preceding switch case! Unless the technique isn't implemented!");
			Storm::throwException<Storm::Exception>("We should have handled the technique to use decision over mesh compute in the preceding switch case! Unless the technique isn't implemented!");
		}
	}
	else
	{
		switch (rbSceneConfig._collisionShape)
		{
		case Storm::CollisionType::Sphere:
			_rbVolume = Storm::AssetVolumeIntegrator::computeSphereVolume(rbSceneConfig._scale);
			break;
		case Storm::CollisionType::Cube:
			_rbVolume = Storm::AssetVolumeIntegrator::computeCubeVolume(rbSceneConfig._scale);
			break;

		case Storm::CollisionType::None:
		case Storm::CollisionType::IndividualParticle:
		case Storm::CollisionType::Custom:
		default:
			Storm::throwException<Storm::Exception>("Cannot fast compute the volume when collision shape is not on a standard shape");
			break;
		}
	}
}
