#include "RigidBody.h"

#include "PoissonDiskSampler.h"
#include "RigidBodySceneData.h"
#include "GeneralSimulationData.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"
#include "IGraphicsManager.h"
#include "IPhysicsManager.h"
#include "ISimulatorManager.h"

#include "MemoryHelper.h"
#include "Version.h"

#include <Assimp\Importer.hpp>
#include <Assimp\scene.h>
#include <Assimp\mesh.h>

#include <boost\algorithm\string.hpp>

#include <fstream>

namespace
{
	std::filesystem::path computeRightCachedFilePath(const Storm::RigidBodySceneData &rbSceneData, const std::filesystem::path &meshPath, float particleRadius)
	{
		std::string suffix;
		suffix.reserve(40);
		suffix += "_x";
		suffix += std::to_string(rbSceneData._scale.x());
		suffix += "_y";
		suffix += std::to_string(rbSceneData._scale.y());
		suffix += "_z";
		suffix += std::to_string(rbSceneData._scale.z());
		suffix += "_rad";
		suffix += std::to_string(particleRadius);

		boost::algorithm::replace_all(suffix, ".", "_");

		const std::filesystem::path meshFileName = std::filesystem::path{ meshPath.stem().string() + suffix }.replace_extension(".cPartRigidBody");

		return Storm::RigidBody::retrieveParticleDataCacheFolder() / meshFileName;
	}
}


Storm::RigidBody::RigidBody(const Storm::RigidBodySceneData &rbSceneData) :
	_meshPath{ rbSceneData._meshFilePath },
	_rbId{ rbSceneData._rigidBodyID }
{
	this->load(rbSceneData);
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

void Storm::RigidBody::load(const Storm::RigidBodySceneData &rbSceneData)
{
	enum : uint64_t
	{
		// Those aren't really checksums but they'll do the same jobs and does it okay (no need to be extra secure, this is not confidential or running stuffs), so I would call them checksum.
		k_cachePlaceholderChecksum = 0x00AA00BB,
		k_cacheGoodChecksum = 0xABCDEF71,
	};

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
	const float currentParticleRadius = configMgr.getGeneralSimulationData()._particleRadius;

	const std::string meshPathLowerStr = boost::algorithm::to_lower_copy(_meshPath);
	const std::filesystem::path meshPath = meshPathLowerStr;
	const std::filesystem::path cachedPath = computeRightCachedFilePath(rbSceneData, meshPath, currentParticleRadius);
	const std::wstring cachedPathStr = cachedPath.wstring();
	constexpr const Storm::Version currentVersion = Storm::Version::retrieveCurrentStormVersion();

	std::vector<Storm::Vector3> verticesPos;
	std::vector<unsigned int> indexes;
	std::vector<Storm::Vector3> particlePos;

	{
		// The mesh is a separate entity, therefore we will load it with Assimp.
		Assimp::Importer meshImporter;
		const aiScene* meshScene = meshImporter.ReadFile(_meshPath, 0);

		if (meshScene->HasMeshes())
		{
			if (meshScene->HasAnimations())
			{
				LOG_WARNING << "'" << _meshPath << "' contains animation. Animations aren't supported so they won't be handled!";
			}
			if (meshScene->HasCameras())
			{
				LOG_WARNING << "'" << _meshPath << "' contains cameras. Embedded cameras aren't supported so they won't be handled!";
			}
			if (meshScene->HasLights())
			{
				LOG_WARNING << "'" << _meshPath << "' contains lights. Lights aren't supported so they won't be handled!";
			}
			if (meshScene->HasMaterials())
			{
				LOG_WARNING << "'" << _meshPath << "' contains materials. Materials aren't supported so they won't be handled!";
			}
			if (meshScene->HasTextures())
			{
				LOG_WARNING << "'" << _meshPath << "' contains textures. Textures aren't supported so they won't be handled!";
			}

			std::size_t totalVertexCount = 0;
			std::size_t totalNormalsCount = 0;
			std::size_t totalIndexesCount = 0;
			for (std::size_t iter = 0; iter < meshScene->mNumMeshes; ++iter)
			{
				const aiMesh* currentMesh = meshScene->mMeshes[iter];
				if (!currentMesh->HasPositions())
				{
					Storm::throwException<std::exception>("The mesh '" + _meshPath + "' doesn't have vertices. This isn't allowed!");
				}
				if (!currentMesh->HasFaces())
				{
					Storm::throwException<std::exception>("The mesh '" + _meshPath + "' doesn't have any faces. This isn't allowed!");
				}

				totalVertexCount += currentMesh->mNumVertices;
				for (std::size_t faceIter = 0; faceIter < currentMesh->mNumFaces; ++faceIter)
				{
					totalIndexesCount += currentMesh->mFaces[faceIter].mNumIndices;
				}
			}

			// Oh my gosh... If you trigger this warning then your object will be awkward to render (I'm not optimizing anything (don't have time) so expect some lags)
			if (totalVertexCount > 10000)
			{
				LOG_WARNING << "'" << _meshPath << "' contains more than 10000 vertices. Low performance, frame drop and high memory consumptions are to be expected. Solution : reduce the number of vertices.";
			}

			verticesPos.reserve(totalVertexCount);

			std::vector<Storm::Vector3> normalsPos;
			normalsPos.reserve(totalVertexCount);

			indexes.reserve(totalIndexesCount);

			for (std::size_t iter = 0; iter < meshScene->mNumMeshes; ++iter)
			{
				const aiMesh* currentMesh = meshScene->mMeshes[iter];

				for (std::size_t verticeIter = 0; verticeIter < currentMesh->mNumVertices; ++verticeIter)
				{
					const aiVector3D &vertice = currentMesh->mVertices[verticeIter];
					verticesPos.emplace_back(vertice.x * rbSceneData._scale.x(), vertice.y * rbSceneData._scale.y(), vertice.z * rbSceneData._scale.z());
				}

				if (currentMesh->mNormals != nullptr)
				{
					for (std::size_t normalsIter = 0; normalsIter < currentMesh->mNumVertices; ++normalsIter)
					{
						const aiVector3D &normals = currentMesh->mNormals[normalsIter];
						normalsPos.emplace_back(normals.x, normals.y, normals.z);
					}
				}
				else
				{
					for (std::size_t normalsIter = 0; normalsIter < currentMesh->mNumVertices; ++normalsIter)
					{
						normalsPos.emplace_back(0.f, 0.f, 0.f);
					}
				}

				for (std::size_t faceIter = 0; faceIter < currentMesh->mNumFaces; ++faceIter)
				{
					const aiFace &currentFace = currentMesh->mFaces[faceIter];
					for (std::size_t indiceIter = 0; indiceIter < currentFace.mNumIndices; ++indiceIter)
					{
						indexes.emplace_back(currentFace.mIndices[indiceIter]);
					}
				}
			}

			Storm::IGraphicsManager &graphicsMgr = singletonHolder.getSingleton<Storm::IGraphicsManager>();
			Storm::IPhysicsManager &physicsMgr = singletonHolder.getSingleton<Storm::IPhysicsManager>();

			graphicsMgr.addMesh(_rbId, verticesPos, normalsPos, indexes);
			physicsMgr.addPhysicalBody(rbSceneData, verticesPos);
		}
		else
		{
			Storm::throwException<std::exception>("The mesh '" + _meshPath + "' doesn't have a mesh inside (an empty mesh). This isn't allowed!");
		}
	}

	const auto srcMeshWriteTime = std::filesystem::last_write_time(meshPath);
	bool hasCache = std::filesystem::exists(cachedPath);
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
		particlePos = Storm::PoissonDiskSampler::process(30, currentParticleRadius, verticesPos);
		
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

	Storm::ISimulatorManager &simulMgr = singletonHolder.getSingleton<Storm::ISimulatorManager>();
	simulMgr.addRigidBodyParticleSystem(_rbId, std::move(particlePos));
}

