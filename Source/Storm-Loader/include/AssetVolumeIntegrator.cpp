#include "AssetVolumeIntegrator.h"

#include "AssetCacheData.h"

#include "VolumeIntegrator.h"

#define XAGORA_SRC_OBJECT_SPACE true
#if !XAGORA_SRC_OBJECT_SPACE
#	include "SceneRigidBodyConfig.h"
#endif


float Storm::AssetVolumeIntegrator::computeTriangleMeshVolume(const Storm::AssetCacheData &mesh)
{
	float result = 0.f;

#if XAGORA_SRC_OBJECT_SPACE
	Storm::Vector3 tetrahedron[3];
#else
	Storm::Vector3 tetrahedron[4];
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
		result += Storm::VolumeIntegrator::computeTetrahedronVolume(tetrahedron);
#else
		result += Storm::VolumeIntegrator::computeTetrahedronVolume(tetrahedron);
#endif
	}

	const float englobingVolume = Storm::VolumeIntegrator::computeCubeVolume(mesh.getFinalBoundingBoxMax() - mesh.getFinalBoundingBoxMin());
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
