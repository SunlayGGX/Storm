#include "RigidBody.h"

#include "PoissonDiskSampler.h"
#include "RigidBodySceneData.h"
#include "GeneralSimulationData.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"
#include "IGraphicsManager.h"

#include "MemoryHelper.h"
#include "Version.h"

#include <Assimp\Importer.hpp>
#include <Assimp\scene.h>

#include <boost\algorithm\string.hpp>

#include <fstream>


Storm::RigidBody::RigidBody(const Storm::RigidBodySceneData &rbSceneData) :
	_meshPath{ rbSceneData._meshFilePath },
	_rbId{ rbSceneData._rigidBodyID }
{
	this->load();
}

const std::string& Storm::RigidBody::getRigidBodyName() const
{
	return _meshPath;
}

unsigned int Storm::RigidBody::getRigidBodyID() const
{
	return _rbId;
}

Storm::Vector3 Storm::RigidBody::getRigidBodyWorldPosition() const
{
	// TODO : Physics module should have the ownership of this data.
	throw std::logic_error("The method or operation is not implemented.");
}

Storm::Vector3 Storm::RigidBody::getRigidBodyWorldRotation() const
{
	// TODO : Physics module should have the ownership of this data.
	throw std::logic_error("The method or operation is not implemented.");
}

Storm::Vector3 Storm::RigidBody::getRigidBodyWorldScale() const
{
	// TODO : Physics module should have the ownership of this data.
	throw std::logic_error("The method or operation is not implemented.");
}

const std::vector<Storm::Vector3>& Storm::RigidBody::getRigidBodyParticlesObjectSpacePositions() const
{
	return _objSpaceParticlePos;
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

void Storm::RigidBody::sampleMesh(const std::vector<Storm::Vector3> &vertices)
{
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();

	// Poisson Disk sampling
	_objSpaceParticlePos = Storm::PoissonDiskSampler{ configMgr.getGeneralSimulationData()._particleRadius, 25 }(vertices);
}

void Storm::RigidBody::load()
{
	enum : uint64_t
	{
		// Those aren't really checksums but they'll do the same jobs and does it okay (no need to be extra secure, this is not confidential or running stuffs), so I would call them checksum.
		k_cachePlaceholderChecksum = 0x00AA00BB,
		k_cacheGoodChecksum = 0xABCDEF71,
	};

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	Storm::IGraphicsManager &graphicsMgr = singletonHolder.getSingleton<Storm::IGraphicsManager>();
	
	const std::string meshPathLowerStr = boost::algorithm::to_lower_copy(_meshPath);
	const std::filesystem::path meshPath = meshPathLowerStr;
	const std::filesystem::path cachedPath = Storm::RigidBody::retrieveParticleDataCacheFolder() / meshPath.stem().replace_extension(".cPartRigidBody");
	const std::wstring cachedPathStr = cachedPath.wstring();
	constexpr const Storm::Version currentVersion = Storm::Version::retrieveCurrentStormVersion();

	std::vector<Storm::Vector3> verticesPos;

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
			for (std::size_t iter = 0; iter < meshScene->mNumMeshes; ++iter)
			{
				const aiMesh* currentMesh = meshScene->mMeshes[iter];
				if (!currentMesh->HasPositions())
				{
					Storm::throwException<std::exception>("The mesh '" + _meshPath + "' doesn't have vertices. This isn't allowed!");
				}

				totalVertexCount += currentMesh->mNumVertices;
			}

			// Oh my gosh... If you trigger this warning then your object will be awkward to render (I'm not optimizing anything (don't have time) so expect some lags)
			if (totalVertexCount > 10000)
			{
				LOG_WARNING << "'" << _meshPath << "' contains more than 10000 vertices. Low performance, frame drop and high memory consumptions are to be expected. Solution : reduce the number of vertices.";
			}

			verticesPos.reserve(totalVertexCount);

			std::vector<Storm::Vector3> normalsPos;
			normalsPos.reserve(totalVertexCount);

			for (std::size_t iter = 0; iter < meshScene->mNumMeshes; ++iter)
			{
				const aiMesh* currentMesh = meshScene->mMeshes[iter];

				for (std::size_t verticeIter = 0; verticeIter < currentMesh->mNumVertices; ++verticeIter)
				{
					const aiVector3D &vertice = currentMesh->mVertices[verticeIter];
					verticesPos.emplace_back(vertice.x, vertice.y, vertice.z);
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
			}

			graphicsMgr.addMesh(_rbId, verticesPos, normalsPos);
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
						LOG_WARNING << '"' << meshPath << "\" has a particle cached file but it is was made with a previous version of the application (no retro compatibility), therefore we will regenerate it anew.";
						hasCache = false;
					}
				}
				else
				{
					LOG_WARNING << '"' << meshPath << "\" has no particle cached file since the one we found was generated for another file that has the same name, therefore we will regenerate it anew.";
					hasCache = false;
				}
			}
			else
			{
				LOG_WARNING << '"' << meshPath << "\" has a particle cached file but it is outdated, therefore we will regenerate it anew.";
				hasCache = false;
			}
		}
		else
		{
			LOG_WARNING << '"' << meshPath << "\" has a particle cached file but it is corrupted (invalid), therefore we will regenerate it anew.";
			hasCache = false;
		}

		if (hasCache)
		{
			LOG_COMMENT << '"' << meshPath << "\" has a right matching particle cached data file, therefore we will load it instead.";
			
			uint64_t particleCount;
			Storm::binaryRead(cacheReadStream, particleCount);
			_objSpaceParticlePos.reserve(particleCount);
			for (uint64_t iter = 0; iter < particleCount; ++iter)
			{
				Storm::Vector3 &currentVect = _objSpaceParticlePos.emplace_back();

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
		
		this->sampleMesh(verticesPos);
		
		/* Cache writing */

		std::ofstream cacheFileStream{ cachedPathStr, std::ios_base::out | std::ios_base::binary };

		// First write wrong checksum as a placeholder
		Storm::binaryWrite(cacheFileStream, static_cast<uint64_t>(k_cachePlaceholderChecksum));

		Storm::binaryWrite(cacheFileStream, static_cast<uint64_t>(srcMeshWriteTime.time_since_epoch().count()));
		Storm::binaryWrite(cacheFileStream, meshPathLowerStr);
		Storm::binaryWrite(cacheFileStream, static_cast<std::string>(currentVersion));

		Storm::binaryWrite(cacheFileStream, static_cast<uint64_t>(_objSpaceParticlePos.size()));
		for (const Storm::Vector3 &particlePos : _objSpaceParticlePos)
		{
			Storm::binaryWrite(cacheFileStream, particlePos.x());
			Storm::binaryWrite(cacheFileStream, particlePos.y());
			Storm::binaryWrite(cacheFileStream, particlePos.z());
		}

		// Replace the placeholder checksum by the right one to finalize the writing
		cacheFileStream.seekp(0);
		Storm::binaryWrite(cacheFileStream, static_cast<uint64_t>(k_cacheGoodChecksum));

		cacheFileStream.flush();
	}
}

