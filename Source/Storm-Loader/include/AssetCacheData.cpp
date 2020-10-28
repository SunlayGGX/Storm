#include "AssetCacheData.h"

#include "RigidBodySceneData.h"

#include "InsideParticleRemovalTechnique.h"
#include "LayeringGenerationTechnique.h"

#include "ThrowException.h"
#include "Vector3Utils.h"
#include "BoundingBox.h"

#define STORM_HIJACKED_TYPE uint32_t
#include "VectHijack.h"

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

	template<bool hasAdditionalLayer, bool isWall>
	void fillVerticesData(const std::vector<Storm::Vector3> &srcVertices, std::vector<Storm::Vector3> &scaledCurrentVertices, const Storm::RigidBodySceneData &rbConfig, std::vector<std::vector<Storm::Vector3>> &scaledAdditionalLayerPosBuffer, const float layerDistance, const aiQuaterniont<Storm::Vector3::Scalar> &rotationQuat, const aiQuaterniont<Storm::Vector3::Scalar> &rotationQuatConjugate, std::vector<Storm::Vector3> &finalCurrentVertices, std::vector<std::vector<Storm::Vector3>> &finalAdditionalLayerPosBuffer, Storm::Vector3 &finalBoundingBoxMin, Storm::Vector3 &finalBoundingBoxMax, const unsigned int additionalLayerCount)
	{
		for (const Storm::Vector3 &vertex : srcVertices)
		{
			// Apply the scale
			const Storm::Vector3 &scaledVertex = scaledCurrentVertices.emplace_back(vertex.x() * rbConfig._scale.x(), vertex.y() * rbConfig._scale.y(), vertex.z() * rbConfig._scale.z());

			if constexpr (hasAdditionalLayer)
			{
				for (unsigned int additionalLayerIndex = 0; additionalLayerIndex < additionalLayerCount; ++additionalLayerIndex)
				{
					const Storm::Vector3 layerPosOffset = scaledVertex.normalized() * (layerDistance * static_cast<float>(additionalLayerIndex + 1));

					Storm::Vector3 &additionalLayerVerticePos = scaledAdditionalLayerPosBuffer[additionalLayerIndex].emplace_back(scaledVertex);
					if constexpr (isWall)
					{
						additionalLayerVerticePos += layerPosOffset;
					}
					else
					{
						additionalLayerVerticePos -= layerPosOffset;
					}
				}
			}

			// Apply the rotation
			const aiQuaterniont<Storm::Vector3::Scalar> tmp{ 0.f, scaledVertex.x(), scaledVertex.y(), scaledVertex.z() };
			const aiQuaterniont<Storm::Vector3::Scalar> tmpRotated{ rotationQuat * tmp * rotationQuatConjugate };

			Storm::Vector3 &finalCurrentVertex = finalCurrentVertices.emplace_back(tmpRotated.x, tmpRotated.y, tmpRotated.z);

			if constexpr (hasAdditionalLayer)
			{
				for (unsigned int layerIndex = 1; layerIndex < rbConfig._layerCount; ++layerIndex)
				{
					Storm::Vector3 &additionalLayerVerticePos = finalAdditionalLayerPosBuffer[layerIndex - 1].emplace_back(finalCurrentVertex);

					const Storm::Vector3 layerPosOffset = finalCurrentVertex.normalized() * (layerDistance * static_cast<float>(layerIndex));
					// The double layer for a wall is outside the rigid body, otherwise it is inside.
					if constexpr (isWall)
					{
						additionalLayerVerticePos += layerPosOffset;
					}
					else
					{
						additionalLayerVerticePos -= layerPosOffset;
					}

					additionalLayerVerticePos += rbConfig._translation;

					if (layerIndex == 1 || layerIndex == additionalLayerCount)
					{
						Storm::minMaxInPlace(finalBoundingBoxMin, finalBoundingBoxMax, additionalLayerVerticePos, [](auto &vect) -> auto& { return vect.x(); });
						Storm::minMaxInPlace(finalBoundingBoxMin, finalBoundingBoxMax, additionalLayerVerticePos, [](auto &vect) -> auto& { return vect.y(); });
						Storm::minMaxInPlace(finalBoundingBoxMin, finalBoundingBoxMax, additionalLayerVerticePos, [](auto &vect) -> auto& { return vect.z(); });
					}
				}
			}

			// Apply the translation
			finalCurrentVertex += rbConfig._translation;

			Storm::minMaxInPlace(finalBoundingBoxMin, finalBoundingBoxMax, finalCurrentVertex, [](auto &vect) -> auto& { return vect.x(); });
			Storm::minMaxInPlace(finalBoundingBoxMin, finalBoundingBoxMax, finalCurrentVertex, [](auto &vect) -> auto& { return vect.y(); });
			Storm::minMaxInPlace(finalBoundingBoxMin, finalBoundingBoxMax, finalCurrentVertex, [](auto &vect) -> auto& { return vect.z(); });
		}
	}

	unsigned int additionalLayer(const unsigned int layerCount)
	{
		return layerCount - 1;
	}

	void initializeTemporaryLayerBuffer(std::vector<std::vector<Storm::Vector3>> &inOutLayersTmpBuffer, const unsigned int additionalLayersCount, const std::size_t verticesCount)
	{
		inOutLayersTmpBuffer.resize(additionalLayersCount);
		for (std::vector<Storm::Vector3> &layerBuff : inOutLayersTmpBuffer)
		{
			layerBuff.reserve(verticesCount);
		}
	}

	template<bool shouldHaveNormals, bool isWall>
	void generateDissociatedTriangleLayersImpl(const Storm::RigidBodySceneData &rbConfig, Storm::AssetCacheData::MeshData &scaledCurrent, Storm::AssetCacheData::MeshData &finalCurrent, const std::vector<uint32_t> &indicesRef, std::vector<uint32_t> &outOverridenIndices, Storm::Vector3 &finalBoundingBoxMin, Storm::Vector3 &finalBoundingBoxMax, const float layerDistance)
	{
		const unsigned int additionalLayersCount = additionalLayer(rbConfig._layerCount);
		const std::size_t verticesCount = finalCurrent._vertices.size();
		const std::size_t srcIndicesCount = indicesRef.size();

		// Since layers triangles will be dissociated, we expect to add as many point as there is indices (triangle point).
		const std::size_t finalVerticesCount = verticesCount + srcIndicesCount * additionalLayersCount;
		scaledCurrent._vertices.reserve(finalVerticesCount);
		finalCurrent._vertices.reserve(finalVerticesCount);

		if constexpr (shouldHaveNormals)
		{
			scaledCurrent._normals.reserve(finalVerticesCount);
			finalCurrent._normals.reserve(finalVerticesCount);
		}

		const std::size_t finalIndicesCount = srcIndicesCount * rbConfig._layerCount;
		outOverridenIndices.reserve(finalIndicesCount);
		Storm::setNumUninitialized_hijack(outOverridenIndices, Storm::VectorHijacker{ finalIndicesCount });
		std::copy(std::begin(indicesRef), std::end(indicesRef), std::begin(outOverridenIndices));
		std::size_t overridenIndicesIndex = srcIndicesCount;

		const auto applyMinMaxBoundingBox = [&finalBoundingBoxMin, &finalBoundingBoxMax](const Storm::Vector3 &finalPosAdded)
		{
			Storm::minMaxInPlace(finalBoundingBoxMin, finalBoundingBoxMax, finalPosAdded, [](auto &vect) -> auto& { return vect.x(); });
			Storm::minMaxInPlace(finalBoundingBoxMin, finalBoundingBoxMax, finalPosAdded, [](auto &vect) -> auto& { return vect.y(); });
			Storm::minMaxInPlace(finalBoundingBoxMin, finalBoundingBoxMax, finalPosAdded, [](auto &vect) -> auto& { return vect.z(); });
		};

		const auto registerAddVertex = [&outOverridenIndices, &scaledCurrent, &finalCurrent, &applyMinMaxBoundingBox, &overridenIndicesIndex](const Storm::Vector3 &triangleScaledVertex, const Storm::Vector3 &triangleFinalVertex, const Storm::Vector3 &triangleScaledOffset, const Storm::Vector3 &triangleFinalOffset)
		{
			outOverridenIndices[overridenIndicesIndex] = static_cast<uint32_t>(scaledCurrent._vertices.size());
			++overridenIndicesIndex;
			if constexpr (isWall)
			{
				scaledCurrent._vertices.emplace_back(triangleScaledVertex + triangleScaledOffset);
				const Storm::Vector3 &finalPosAdded = finalCurrent._vertices.emplace_back(triangleFinalVertex + triangleFinalOffset);
				applyMinMaxBoundingBox(finalPosAdded);
			}
			else
			{
				scaledCurrent._vertices.emplace_back(triangleScaledVertex - triangleScaledOffset);
				const Storm::Vector3 &finalPosAdded = finalCurrent._vertices.emplace_back(triangleFinalVertex - triangleFinalOffset);
				applyMinMaxBoundingBox(finalPosAdded);
			}
		};

		for (std::size_t iter0 = 0; iter0 < srcIndicesCount; iter0 += 3)
		{
			const std::size_t iter1 = iter0 + 1;
			const std::size_t iter2 = iter0 + 2;

			const uint32_t indice0 = indicesRef[iter0];
			const uint32_t indice1 = indicesRef[iter1];
			const uint32_t indice2 = indicesRef[iter2];

			const Storm::Vector3 &triangleScaledVertex0 = scaledCurrent._vertices[indice0];
			const Storm::Vector3 &triangleScaledVertex1 = scaledCurrent._vertices[indice1];
			const Storm::Vector3 &triangleScaledVertex2 = scaledCurrent._vertices[indice2];

			const Storm::Vector3 &triangleFinalVertex0 = finalCurrent._vertices[indice0];
			const Storm::Vector3 &triangleFinalVertex1 = finalCurrent._vertices[indice1];
			const Storm::Vector3 &triangleFinalVertex2 = finalCurrent._vertices[indice2];

			Storm::Vector3 triangleScaledNormals = (triangleScaledVertex1 - triangleScaledVertex0).cross(triangleScaledVertex2 - triangleScaledVertex0);
			triangleScaledNormals.normalize();

			Storm::Vector3 triangleFinalNormals = (triangleFinalVertex1 - triangleFinalVertex0).cross(triangleFinalVertex2 - triangleFinalVertex0);
			triangleFinalNormals.normalize();

			for (unsigned int layerIndex = 1; layerIndex < rbConfig._layerCount; ++layerIndex)
			{
				const float layerOffset = layerDistance * static_cast<float>(layerIndex);

				const Storm::Vector3 triangleScaledOffset = triangleScaledNormals * layerOffset;
				const Storm::Vector3 triangleFinalOffset = triangleFinalNormals * layerOffset;

				registerAddVertex(triangleScaledVertex0, triangleFinalVertex0, triangleScaledOffset, triangleFinalOffset);
				registerAddVertex(triangleScaledVertex1, triangleFinalVertex1, triangleScaledOffset, triangleFinalOffset);
				registerAddVertex(triangleScaledVertex2, triangleFinalVertex2, triangleScaledOffset, triangleFinalOffset);

				if constexpr (shouldHaveNormals)
				{
					scaledCurrent._normals.emplace_back(triangleScaledNormals);
					scaledCurrent._normals.emplace_back(triangleScaledNormals);
					scaledCurrent._normals.emplace_back(triangleScaledNormals);

					finalCurrent._normals.emplace_back(triangleFinalNormals);
					finalCurrent._normals.emplace_back(triangleFinalNormals);
					finalCurrent._normals.emplace_back(triangleFinalNormals);
				}
			}
		}
	}
}


Storm::AssetCacheData::AssetCacheData(const Storm::RigidBodySceneData &rbConfig, const aiScene* meshScene, const float layerDistance) :
	_rbConfig{ rbConfig },
	_src{ std::make_shared<Storm::AssetCacheData::MeshData>() },
	_indices{ std::make_shared<std::vector<uint32_t>>() },
	_finalBoundingBoxMin{ Storm::initVector3ForMin() },
	_finalBoundingBoxMax{ Storm::initVector3ForMax() }
{
	this->buildSrc(meshScene);
	this->generateCurrentData(layerDistance);
}

Storm::AssetCacheData::AssetCacheData(const Storm::RigidBodySceneData &rbConfig, const Storm::AssetCacheData &srcCachedData, const float layerDistance) :
	_rbConfig{ rbConfig },
	_src{ srcCachedData._src },
	_indices{ srcCachedData._indices }
{
	this->generateCurrentData(layerDistance);
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
				_rbConfig._rotation == rbConfig._rotation &&
				_rbConfig._layerCount == rbConfig._layerCount
			)
		);
}

bool Storm::AssetCacheData::isInsideFinalBoundingBox(const Storm::Vector3 &pos) const
{
	return Storm::isInsideBoundingBox(_finalBoundingBoxMin, _finalBoundingBoxMax, pos);
}

const Storm::Vector3& Storm::AssetCacheData::getFinalBoundingBoxMin() const noexcept
{
	return _finalBoundingBoxMin;
}

const Storm::Vector3& Storm::AssetCacheData::getFinalBoundingBoxMax() const noexcept
{
	return _finalBoundingBoxMax;
}

void Storm::AssetCacheData::removeInsiderParticle(std::vector<Storm::Vector3> &inOutParticles) const
{
	const std::size_t exParticleCount = inOutParticles.size();
	bool hasRunRemovalAlgorithm = true;

	switch (_rbConfig._insideRbFluidDetectionMethodEnum)
	{
	case Storm::InsideParticleRemovalTechnique::Normals:
		this->removeInsiderParticleWithNormalsMethod(inOutParticles);
		break;

	default:
		hasRunRemovalAlgorithm = false;
		break;
	}

	if (hasRunRemovalAlgorithm)
	{
		const std::size_t nowParticleCount = inOutParticles.size();
		if (exParticleCount != nowParticleCount)
		{
			LOG_DEBUG << exParticleCount - nowParticleCount << " particle(s) were removed because they were inside rigid body " << _rbConfig._rigidBodyID;
		}
	}
}

void Storm::AssetCacheData::removeInsiderParticleWithNormalsMethod(std::vector<Storm::Vector3> &inOutParticles) const
{
	LOG_DEBUG << "Remove insider particles from rigid body " << _rbConfig._rigidBodyID << " using normals technique.";

	const std::size_t normalCount = _finalCurrent._normals.size();

	assert(_finalCurrent._vertices.size() == normalCount && "Vertices count mismatch Normals count!");

	auto eraseIt = std::partition(std::execution::par, std::begin(inOutParticles), std::end(inOutParticles), [this, normalCount](const Storm::Vector3 &particlePos)
	{
		if (this->isInsideFinalBoundingBox(particlePos))
		{
			Storm::Vector3 relativePosToVertex;

			for (std::size_t iter = 0; iter < normalCount; iter += 3)
			{
				const Storm::Vector3 &consideredVertex = _finalCurrent._vertices[iter];

				relativePosToVertex.x() = consideredVertex.x() - particlePos.x();
				relativePosToVertex.y() = consideredVertex.y() - particlePos.y();
				relativePosToVertex.z() = consideredVertex.z() - particlePos.z();

				// The point is in front of the face, therefore it is outside.
				if (relativePosToVertex.dot(_finalCurrent._normals[iter]) < 0.f)
				{
					return true;
				}
			}

			return false;
		}

		return true;
	});

	for (std::size_t toRemove = std::end(inOutParticles) - eraseIt; toRemove > 0; --toRemove)
	{
		inOutParticles.pop_back();
	}
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
	return _overrideIndices.empty() ? *_indices : _overrideIndices;
}

const Storm::RigidBodySceneData& Storm::AssetCacheData::getAssociatedRbConfig() const noexcept
{
	return _rbConfig;
}

void Storm::AssetCacheData::generateCurrentData(const float layerDistance)
{
	const std::vector<Storm::Vector3> &srcVertices = _src->_vertices;
	const std::vector<Storm::Vector3> &srcNormals = _src->_normals;

	const std::size_t srcVerticesCount = srcVertices.size();
	const std::size_t srcNormalsCount = srcNormals.size();

	const std::size_t verticeCount = srcVerticesCount * _rbConfig._layerCount;
	const std::size_t normalsCount = srcNormalsCount * _rbConfig._layerCount;
	_scaledCurrent._vertices.reserve(verticeCount);
	_scaledCurrent._normals.reserve(normalsCount);
	_finalCurrent._vertices.reserve(verticeCount);
	_finalCurrent._normals.reserve(normalsCount);

	const aiQuaterniont<Storm::Vector3::Scalar> rotationQuat{ _rbConfig._rotation.x(), _rbConfig._rotation.y(), _rbConfig._rotation.z() };
	aiQuaterniont<Storm::Vector3::Scalar> rotationQuatConjugate = rotationQuat;
	rotationQuatConjugate.Conjugate();

	const unsigned int additionalLayersCount = additionalLayer(_rbConfig._layerCount);

#define STORM_FILL_VERTICES_DATA_ARGUMENTS \
	srcVertices,						   \
	_scaledCurrent._vertices,			   \
	_rbConfig,							   \
	scaledAdditionalLayerPosBuffer,		   \
	layerDistance,						   \
	rotationQuat,						   \
	rotationQuatConjugate,				   \
	_finalCurrent._vertices,			   \
	finalAdditionalLayerPosBuffer,		   \
	_finalBoundingBoxMin,				   \
	_finalBoundingBoxMax,				   \
	additionalLayersCount

	std::vector<std::vector<Storm::Vector3>> scaledAdditionalLayerPosBuffer;
	std::vector<std::vector<Storm::Vector3>> finalAdditionalLayerPosBuffer;
	if (additionalLayersCount > 0 && _rbConfig._layerGenerationMode == Storm::LayeringGenerationTechnique::Scaling)
	{
		initializeTemporaryLayerBuffer(scaledAdditionalLayerPosBuffer, additionalLayersCount, srcVerticesCount);
		initializeTemporaryLayerBuffer(finalAdditionalLayerPosBuffer, additionalLayersCount, srcVerticesCount);

		if (_rbConfig._isWall)
		{
			fillVerticesData<true, true>(STORM_FILL_VERTICES_DATA_ARGUMENTS);
		}
		else
		{
			fillVerticesData<true, false>(STORM_FILL_VERTICES_DATA_ARGUMENTS);
		}
	}
	else
	{
		fillVerticesData<false, false>(STORM_FILL_VERTICES_DATA_ARGUMENTS);
	}

#undef STORM_FILL_VERTICES_DATA_ARGUMENTS

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

	if (additionalLayersCount > 0)
	{
		switch (_rbConfig._layerGenerationMode)
		{
		case Storm::LayeringGenerationTechnique::Scaling:
		{
			const std::size_t layerVerticeCount = finalAdditionalLayerPosBuffer[0].size();

			for (unsigned int layerIter = 0; layerIter < additionalLayersCount; ++layerIter)
			{
				const std::vector<Storm::Vector3> &scaledPosLayerBuffer = scaledAdditionalLayerPosBuffer[layerIter];
				const std::vector<Storm::Vector3> &finalPosLayerBuffer = finalAdditionalLayerPosBuffer[layerIter];

				for (std::size_t iter = 0; iter < layerVerticeCount; ++iter)
				{
					_scaledCurrent._vertices.emplace_back(scaledPosLayerBuffer[iter]);
					_scaledCurrent._normals.emplace_back(_scaledCurrent._normals[iter]);
					_finalCurrent._vertices.emplace_back(finalPosLayerBuffer[iter]);
					_finalCurrent._normals.emplace_back(_finalCurrent._normals[iter]);
				}
			}

			std::vector<uint32_t> indicesRef = *_indices;
			const std::size_t indicesCount = indicesRef.size();

			_overrideIndices.resize(indicesCount * _rbConfig._layerCount);
			std::copy(std::begin(indicesRef), std::end(indicesRef), std::begin(_overrideIndices));

			for (std::size_t layerIndex = 1; layerIndex < _rbConfig._layerCount; ++layerIndex)
			{
				const std::size_t layerIndexOffset = indicesCount * layerIndex;
				const uint32_t layerPosIndiceOffset = static_cast<uint32_t>(layerVerticeCount * layerIndex);
				for (std::size_t iter = 0; iter < indicesCount; ++iter)
				{
					_overrideIndices[iter + layerIndexOffset] = indicesRef[iter] + layerPosIndiceOffset;
				}
			}
		}
		break;

		case Storm::LayeringGenerationTechnique::DissociatedTriangle:
			this->generateDissociatedTriangleLayers(layerDistance);
			break;

		default:
			Storm::throwException<std::exception>("Unknown layering generation technique : " + Storm::toStdString(_rbConfig._layerGenerationMode));
		}
	}

	// Add a little margin to the bounding box to avoid making strict equality afterwards and ensure that the bounding box encloses the shape
	// (because currently, the point used to compute this bounding box is right on the skin of the box, so not enclosed).
	constexpr float margin = 0.0000001f;
	_finalBoundingBoxMin.x() -= margin;
	_finalBoundingBoxMin.y() -= margin;
	_finalBoundingBoxMin.z() -= margin;
	_finalBoundingBoxMax.x() += margin;
	_finalBoundingBoxMax.y() += margin;
	_finalBoundingBoxMax.z() += margin;
}

void Storm::AssetCacheData::buildSrc(const aiScene* meshScene)
{
	std::vector<Storm::Vector3> &verticesPos = _src->_vertices;
	std::vector<Storm::Vector3> &normalsPos = _src->_normals;
	std::vector<uint32_t> &indices = *_indices;

	std::size_t totalVertexCount = 0;
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

	const bool shouldGenerateNormalsIfNot = _rbConfig._insideRbFluidDetectionMethodEnum == InsideParticleRemovalTechnique::Normals;

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

		for (std::size_t faceIter = 0; faceIter < currentMesh->mNumFaces; ++faceIter)
		{
			const aiFace &currentFace = currentMesh->mFaces[faceIter];
			for (std::size_t indiceIter = 0; indiceIter < currentFace.mNumIndices; ++indiceIter)
			{
				indices.emplace_back(currentFace.mIndices[indiceIter]);
			}
		}
	}

	if (normalsPos.size() != verticesPos.size())
	{
		if (shouldGenerateNormalsIfNot)
		{
			LOG_WARNING << "Rigid body " << _rbConfig._rigidBodyID << " doesn't have normals but we needs them (i.e to remove insider particles). Therefore we will generate them.";

			for (std::size_t indicesIter = normalsPos.size(); indicesIter < totalIndexesCount; indicesIter += 3)
			{
				const Storm::Vector3 &p1 = verticesPos[indices[indicesIter]];
				const Storm::Vector3 &p2 = verticesPos[indices[indicesIter + 1]];

				const Storm::Vector3 normal = p1.cross(p2);
				normalsPos.emplace_back(normal);
				normalsPos.emplace_back(normal);
				normalsPos.emplace_back(normal);
			}
		}
		else
		{
			for (std::size_t normalsIter = normalsPos.size(); normalsIter < totalVertexCount; ++normalsIter)
			{
				normalsPos.emplace_back(0.f, 0.f, 0.f);
			}
		}
	}

	assert(normalsPos.size() == verticesPos.size() && "Vertices count mismatch Normals count!");
}

void Storm::AssetCacheData::generateDissociatedTriangleLayers(const float layerDistance)
{
	if (_scaledCurrent._normals.empty())
	{
		if (_rbConfig._isWall)
		{
			generateDissociatedTriangleLayersImpl<false, true>(_rbConfig,
				_scaledCurrent, _finalCurrent,
				*_indices, _overrideIndices,
				_finalBoundingBoxMin, _finalBoundingBoxMax,
				layerDistance
			);
		}
		else
		{
			generateDissociatedTriangleLayersImpl<false, false>(_rbConfig,
				_scaledCurrent, _finalCurrent,
				*_indices, _overrideIndices,
				_finalBoundingBoxMin, _finalBoundingBoxMax,
				layerDistance
			);
		}
	}
	else
	{
		if (_rbConfig._isWall)
		{
			generateDissociatedTriangleLayersImpl<true, true>(_rbConfig,
				_scaledCurrent, _finalCurrent,
				*_indices, _overrideIndices,
				_finalBoundingBoxMin, _finalBoundingBoxMax,
				layerDistance
			);
		}
		else
		{
			generateDissociatedTriangleLayersImpl<true, false>(_rbConfig,
				_scaledCurrent, _finalCurrent,
				*_indices, _overrideIndices,
				_finalBoundingBoxMin, _finalBoundingBoxMax,
				layerDistance
			);
		}
	}
}
