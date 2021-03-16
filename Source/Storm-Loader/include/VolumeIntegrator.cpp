#include "VolumeIntegrator.h"

#include "AssetCacheData.h"

#define XAGORA_SRC_OBJECT_SPACE true
#if !XAGORA_SRC_OBJECT_SPACE
#	include "SceneRigidBodyConfig.h"
#endif

namespace
{
	template<bool shouldConsiderLastAsCenter>
	float computeTetrahedronVolumeImpl(const Storm::Vector3(&vertexes)[4])
	{
		float tmp;
		if constexpr (shouldConsiderLastAsCenter)
		{
			const Storm::Vector3 v30 = vertexes[0] - vertexes[3];
			const Storm::Vector3 v20 = vertexes[1] - vertexes[3];
			const Storm::Vector3 v10 = vertexes[2] - vertexes[3];
			tmp = std::fabs(v30.cross(v20).dot(v10));
		}
		else // Center at 0.0, so no need to subtract.
		{
			const Storm::Vector3 &v30 = vertexes[0];
			const Storm::Vector3 &v20 = vertexes[1];
			const Storm::Vector3 &v10 = vertexes[2];
			tmp = std::fabs(v30.cross(v20).dot(v10));
		}
		return  tmp / 6.f;
	}
}


float Storm::VolumeIntegrator::computeSphereVolume(const Storm::Vector3 &dimension)
{
	if (dimension.x() != dimension.y() || dimension.x() != dimension.z())
	{
		assert(false && "Spheroids aren't handled for now!");
		Storm::throwException<Storm::Exception>("Spheroids aren't handled for now!");
	}

	const double radius = static_cast<double>(dimension.x()) / 2.0;
	constexpr double k_coeff = 4.0 / 3.0 * M_PI;

	return static_cast<float>(k_coeff * radius * radius * radius);
}

float Storm::VolumeIntegrator::computeCubeVolume(const Storm::Vector3 &dimension)
{
	return std::fabs(dimension.x() * dimension.y() * dimension.z());
}

float Storm::VolumeIntegrator::computeTetrahedronVolume(const Storm::Vector3(&vertexes)[4])
{
	return computeTetrahedronVolumeImpl<true>(vertexes);
}

float Storm::VolumeIntegrator::computeTriangleMeshVolume(const Storm::AssetCacheData &mesh)
{
	float result = 0.f;

	Storm::Vector3 tetrahedron[4];

#if !XAGORA_SRC_OBJECT_SPACE
	tetrahedron[3] = mesh.getAssociatedRbConfig()._translation;
#endif

	const std::vector<Storm::Vector3> &vertices = mesh.getScaledVertices();
	const std::vector<uint32_t> &indices = mesh.getSrcIndices();
	const std::size_t indicesCount = indices.size();

	for (std::size_t iter = 0; iter < indicesCount; iter += 3)
	{
		tetrahedron[0] = vertices[indices[iter]];
		tetrahedron[1] = vertices[indices[iter + 1]];
		tetrahedron[2] = vertices[indices[iter + 2]];

#if XAGORA_SRC_OBJECT_SPACE
		result += computeTetrahedronVolumeImpl<false>(tetrahedron);
#else
		result += computeTetrahedronVolumeImpl<true>(tetrahedron);
#endif
	}

	const float englobingVolume = computeCubeVolume(mesh.getFinalBoundingBoxMax() - mesh.getFinalBoundingBoxMin());
	if (result == 0.f && englobingVolume == 0.f)
	{
		LOG_WARNING << "Volume was computed as empty!";
	}
	else if (result == 0.f)
	{
		result = englobingVolume;
	}
	else if (englobingVolume != 0.f)
	{
		result = std::min(result, englobingVolume);
	}

	return result;
}
