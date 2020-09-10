#include "AssetCacheData.h"

#include "RigidBodySceneData.h"

#include "ThrowException.h"

#include <Assimp\scene.h>
#include <Assimp\mesh.h>


namespace
{
	class PrimitiveTypeParser
	{
	public:
		template<class PolicyType>
		static std::string parse(aiPrimitiveType primitive)
		{
			switch (primitive)
			{
			case aiPrimitiveType::aiPrimitiveType_POINT: return "Point";
			case aiPrimitiveType::aiPrimitiveType_LINE: return "Line";
			case aiPrimitiveType::aiPrimitiveType_TRIANGLE: return "Triangle";
			case aiPrimitiveType::aiPrimitiveType_POLYGON: return "Polygon like Quads (not triangle)";

			case aiPrimitiveType::_aiPrimitiveType_Force32Bit:
			default:
				return "Unknown primitive type";
			}
		}
	};
}


Storm::AssetCacheData::AssetCacheData(const Storm::RigidBodySceneData &rbConfig, const aiScene* meshScene) :
	_rbConfig{ rbConfig },
	_src{ std::make_shared<Storm::AssetCacheData::MeshData>() },
	_indices{ std::make_shared<std::vector<uint32_t>>() }
{
	this->buildSrc(meshScene);
	this->generateCurrentData();
}

Storm::AssetCacheData::AssetCacheData(const Storm::RigidBodySceneData &rbConfig, const Storm::AssetCacheData &srcCachedData) :
	_rbConfig{ rbConfig },
	_src{ srcCachedData._src },
	_indices{ srcCachedData._indices }
{
	this->generateCurrentData();
}

bool Storm::AssetCacheData::isEquivalentWith(const Storm::RigidBodySceneData &rbConfig, bool considerFinal) const
{
	return
		_rbConfig._meshFilePath == rbConfig._meshFilePath &&
		_rbConfig._scale == rbConfig._scale &&
		(
			!considerFinal ||
			(
				_rbConfig._translation == rbConfig._translation &&
				_rbConfig._rotation == rbConfig._rotation
			)
		);
}

const std::vector<Storm::Vector3>& Storm::AssetCacheData::getSrcVertices() const noexcept
{
	return _src->_vertices;
}

const std::vector<Storm::Vector3>& Storm::AssetCacheData::getSrcNormals() const noexcept
{
	return _src->_normals;
}

const std::vector<Storm::Vector3>& Storm::AssetCacheData::getScaledVertices() const noexcept
{
	return _scaledCurrent._vertices;
}

const std::vector<Storm::Vector3>& Storm::AssetCacheData::getScaledNormals() const noexcept
{
	return _scaledCurrent._normals;
}

const std::vector<Storm::Vector3>& Storm::AssetCacheData::getFinalVertices() const noexcept
{
	return _finalCurrent._vertices;
}

const std::vector<Storm::Vector3>& Storm::AssetCacheData::getFinalNormals() const noexcept
{
	return _finalCurrent._normals;
}

const std::vector<uint32_t>& Storm::AssetCacheData::getIndices() const noexcept
{
	return *_indices;
}

void Storm::AssetCacheData::generateCurrentData()
{
	const std::vector<Storm::Vector3> &srcVertices = _src->_vertices;
	const std::vector<Storm::Vector3> &srcNormals = _src->_normals;

	const std::size_t srcVerticesCount = srcVertices.size();
	const std::size_t srcNormalsCount = srcNormals.size();

	_scaledCurrent._vertices.reserve(srcVerticesCount);
	_scaledCurrent._normals.reserve(srcNormalsCount);
	_finalCurrent._vertices.reserve(srcVerticesCount);
	_finalCurrent._normals.reserve(srcNormalsCount);

	const aiQuaterniont<Storm::Vector3::Scalar> rotationQuat{ _rbConfig._rotation.x(), _rbConfig._rotation.y(), _rbConfig._rotation.z() };
	aiQuaterniont<Storm::Vector3::Scalar> rotationQuatConjugate = rotationQuat;
	rotationQuatConjugate.Conjugate();

	for (const Storm::Vector3 &vertex : srcVertices)
	{
		// Apply the scale
		const Storm::Vector3 &scaledVertex = _scaledCurrent._vertices.emplace_back(vertex.x() * _rbConfig._scale.x(), vertex.y() * _rbConfig._scale.y(), vertex.z() * _rbConfig._scale.z());

		// Apply the rotation
		const aiQuaterniont<Storm::Vector3::Scalar> tmp{ 0.f, scaledVertex.x(), scaledVertex.y(), scaledVertex.z() };
		const aiQuaterniont<Storm::Vector3::Scalar> tmpRotated{ rotationQuat * tmp * rotationQuatConjugate };
		
		Storm::Vector3 &finalCurrentVertex = _finalCurrent._vertices.emplace_back(tmpRotated.x, tmpRotated.y, tmpRotated.z);

		// Apply the translation
		finalCurrentVertex += _rbConfig._translation;
	}

	for (const Storm::Vector3 &normal : srcNormals)
	{
		// Apply the scale
		Storm::Vector3 &scaledNormal = _scaledCurrent._normals.emplace_back(normal.x() * _rbConfig._scale.x(), normal.y() * _rbConfig._scale.y(), normal.z() * _rbConfig._scale.z());
		scaledNormal.normalize();

		// Apply the rotation
		const aiQuaterniont<Storm::Vector3::Scalar> tmp{ 0.f, scaledNormal.x(), scaledNormal.y(), scaledNormal.z() };
		const aiQuaterniont<Storm::Vector3::Scalar> tmpRotated{ rotationQuat * tmp * rotationQuatConjugate };

		_finalCurrent._normals.emplace_back(tmpRotated.x, tmpRotated.y, tmpRotated.z);

		// This is a vector (not a point) so no translation.
	}
}

void Storm::AssetCacheData::buildSrc(const aiScene* meshScene)
{
	std::vector<Storm::Vector3> &verticesPos = _src->_vertices;
	std::vector<Storm::Vector3> &normalsPos = _src->_normals;
	std::vector<uint32_t> &indices = *_indices;

	std::size_t totalVertexCount = 0;
	std::size_t totalNormalsCount = 0;
	std::size_t totalIndexesCount = 0;
	for (std::size_t iter = 0; iter < meshScene->mNumMeshes; ++iter)
	{
		const aiMesh* currentMesh = meshScene->mMeshes[iter];
		if (!currentMesh->HasPositions())
		{
			Storm::throwException<std::exception>("The mesh '" + _rbConfig._meshFilePath + "' doesn't have vertices. This isn't allowed!");
		}
		else if (!currentMesh->HasFaces())
		{
			Storm::throwException<std::exception>("The mesh '" + _rbConfig._meshFilePath + "' doesn't have any faces. This isn't allowed!");
		}
		else if (currentMesh->mPrimitiveTypes != aiPrimitiveType::aiPrimitiveType_TRIANGLE)
		{
			Storm::throwException<std::exception>("The mesh '" + _rbConfig._meshFilePath + "' isn't constituted of triangles. We doesn't support non triangle meshes! Primitive type was '" + Storm::toStdString<PrimitiveTypeParser>(static_cast<aiPrimitiveType>(currentMesh->mPrimitiveTypes)) + "'");
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
		LOG_WARNING << "'" << _rbConfig._meshFilePath << "' contains more than 10000 vertices. Low performance, frame drop and high memory consumptions are to be expected. Solution : reduce the number of vertices.";
	}

	verticesPos.reserve(totalVertexCount);
	normalsPos.reserve(totalVertexCount);
	indices.reserve(totalIndexesCount);

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

		for (std::size_t faceIter = 0; faceIter < currentMesh->mNumFaces; ++faceIter)
		{
			const aiFace &currentFace = currentMesh->mFaces[faceIter];
			for (std::size_t indiceIter = 0; indiceIter < currentFace.mNumIndices; ++indiceIter)
			{
				indices.emplace_back(currentFace.mIndices[indiceIter]);
			}
		}
	}
}
