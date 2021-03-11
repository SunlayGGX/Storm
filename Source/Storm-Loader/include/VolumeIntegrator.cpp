#include "VolumeIntegrator.h"

#include "AssetCacheData.h"

#define XAGORA_SRC_OBJECT_SPACE true
#if !XAGORA_SRC_OBJECT_SPACE
#	include "SceneRigidBodyConfig.h"
#endif


float Storm::VolumeIntegrator::computeSphereVolume(const Storm::Vector3 &dimension)
{
	if (dimension.x() != dimension.y() || dimension.x() != dimension.z())
	{
		assert(false && "Spheroids aren't handled for now!");
		Storm::throwException<Storm::Exception>("Spheroids aren't handled for now!");
	}

	const double radius = static_cast<double>(dimension.x());
	constexpr double k_coeff = 4.0 / 3.0 * M_PI;

	return static_cast<float>(k_coeff * radius * radius * radius);
}

float Storm::VolumeIntegrator::computeCubeVolume(const Storm::Vector3 &dimension)
{
	return dimension.x() * dimension.y() * dimension.z();
}

float Storm::VolumeIntegrator::computeTetrahedronVolume(const Storm::Vector3(&vertexes)[4])
{
	constexpr float k_coeff = 1.f / 6.f;
	const Storm::Vector3 v01 = vertexes[1] - vertexes[0];
	const Storm::Vector3 v02 = vertexes[2] - vertexes[0];
	const Storm::Vector3 v03 = vertexes[3] - vertexes[0];

	return k_coeff * std::fabs(v01.cross(v02).dot(v03));
}

float Storm::VolumeIntegrator::computeTriangleMeshVolume(const Storm::AssetCacheData &mesh)
{
	float result = 0.f;

	Storm::Vector3 tetrahedron[4];

#if XAGORA_SRC_OBJECT_SPACE
	tetrahedron[0] = Storm::Vector3::Zero();
#else
	tetrahedron[0] = mesh.getAssociatedRbConfig()._translation;
#endif

	const std::vector<Storm::Vector3> &vertices = mesh.getSrcVertices();
	const std::vector<uint32_t> &indices = mesh.getIndices();
	const std::size_t indicesCount = indices.size();

	for (std::size_t iter = 0; iter < indicesCount; iter += 3)
	{
		tetrahedron[1] = vertices[indices[iter]];
		tetrahedron[2] = vertices[indices[iter + 1]];
		tetrahedron[3] = vertices[indices[iter + 2]];

		result += Storm::VolumeIntegrator::computeTetrahedronVolume(tetrahedron);
	}

	if (result == 0.f)
	{
		LOG_WARNING << "Volume was computed as empty!";
	}

	return result;
}
