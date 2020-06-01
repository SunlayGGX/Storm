#include "RigidBody.h"

#include "RigidBodySceneData.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "MemoryHelper.h"

#include <Assimp\Importer.hpp>
#include <Assimp\scene.h>

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

void Storm::RigidBody::load()
{
	enum : uint64_t
	{
		k_cacheFirstChecksum = 0x00AA00BB,
		k_cacheGoodChecksum = 0xABCDEF71,
	};

	const Storm::IConfigManager*const configMgr = Storm::SingletonHolder::instance().getFacet<Storm::IConfigManager>();
	
	const std::filesystem::path meshPath = _meshPath;
	const std::filesystem::path tmpPath = configMgr->getTemporaryPath();
	const std::filesystem::path cachedPath = tmpPath / meshPath.stem().replace_extension(".cPartRigidBody");

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
			for (std::size_t iter = 0; iter < meshScene->mNumMeshes; ++iter)
			{
				const aiMesh* currentMesh = meshScene->mMeshes[iter];
				if (!currentMesh->HasPositions())
				{
					Storm::throwException<std::exception>("The mesh '" + _meshPath + "' doesn't have vertices. This isn't allowed!");
				}

				totalVertexCount += currentMesh->mNumVertices;
			}

			if (totalVertexCount > 50000)
			{
				LOG_WARNING << "'" << _meshPath << "' contains more than 50000 vertices. Low performance, frame drop and high memory consumptions are to be expected. Solution : reduce the number of vertices.";
			}

			std::vector<Storm::Vector3> verticesPos;
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

				for (std::size_t normalsIter = 0; normalsIter < currentMesh->mNumVertices; ++normalsIter)
				{
					const aiVector3D &normals = currentMesh->mNormals[normalsIter];
					normalsPos.emplace_back(normals.x, normals.y, normals.z);
				}
			}

			// TODO : Pass the data to the Graphics module.
		}
		else
		{
			Storm::throwException<std::exception>("The mesh '" + _meshPath + "' doesn't have a mesh inside (an empty mesh). This isn't allowed!");
		}
	}

	bool hasCache = std::filesystem::exists(cachedPath);
	if (hasCache)
	{
		std::ifstream cacheReadStream{ cachedPath.wstring(), std::ios_base::in | std::ios_base::binary };

		uint64_t checksum;
		Storm::binaryRead(cacheReadStream, checksum);
		if (checksum == k_cacheGoodChecksum)
		{
			int64_t timestamp;
			Storm::binaryRead(cacheReadStream, timestamp);

			if (std::filesystem::file_time_type{ std::filesystem::file_time_type::duration{ timestamp } } == std::filesystem::last_write_time(meshPath))
			{
				LOG_COMMENT << '"' << meshPath << "\" has a particle cached file, therefore we will load it instead.";

				// TODO : read the particle saved data

			}
			else
			{
				LOG_WARNING << '"' << meshPath << "\" has a particle cached file but it is outdated, therefore we will regenerate it.";
				hasCache = false;
			}
		}
		else
		{
			LOG_WARNING << '"' << meshPath << "\" has a particle cached file but it is corrupted (invalid), therefore we will regenerate it.";
			hasCache = false;
		}

		if(!hasCache)
		{
			cacheReadStream.close();
			std::filesystem::remove_all(cachedPath);
		}
	}

	if(!hasCache)
	{
		// TODO : Generate the rb as if no cache data

		// TODO : save the particle cache file.
	}
}

