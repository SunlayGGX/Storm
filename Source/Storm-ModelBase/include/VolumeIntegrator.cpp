#include "VolumeIntegrator.h"

namespace
{
	template<std::size_t tetrahedronVertexCount>
	float computeTetrahedronVolumeImpl(const Storm::Vector3(&vertexes)[tetrahedronVertexCount])
	{
		float tmp;

		if constexpr (tetrahedronVertexCount == 4)
		{
			const Storm::Vector3 v30 = vertexes[0] - vertexes[3];
			const Storm::Vector3 v20 = vertexes[1] - vertexes[3];
			const Storm::Vector3 v10 = vertexes[2] - vertexes[3];
			tmp = std::fabs(v30.cross(v20).dot(v10));
		}
		else if constexpr (tetrahedronVertexCount == 3) // Center at 0.0, so no need to subtract.
		{
			const Storm::Vector3 &v30 = vertexes[0];
			const Storm::Vector3 &v20 = vertexes[1];
			const Storm::Vector3 &v10 = vertexes[2];
			tmp = std::fabs(v30.cross(v20).dot(v10));
		}
		else
		{
			STORM_COMPILE_ERROR("Tetrahedron vertex count is not accepted! Must be either 4 or 3 (in case of 3, the 4th vertex is 0).");
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

	return static_cast<float>(std::fabs(k_coeff * radius * radius * radius));
}

float Storm::VolumeIntegrator::computeCubeVolume(const Storm::Vector3 &dimension)
{
	return std::fabs(dimension.x() * dimension.y() * dimension.z());
}

float Storm::VolumeIntegrator::computeTetrahedronVolume(const Storm::Vector3(&vertexes)[4])
{
	return computeTetrahedronVolumeImpl(vertexes);
}

float Storm::VolumeIntegrator::computeTetrahedronVolume(const Storm::Vector3(&vertexes)[3])
{
	return computeTetrahedronVolumeImpl(vertexes);
}
