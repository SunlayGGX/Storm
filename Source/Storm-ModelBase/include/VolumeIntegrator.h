#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	class VolumeIntegrator : private Storm::NonInstanciable
	{
	public:
		static float computeSphereVolume(const Storm::Vector3 &dimension);
		static float computeCubeVolume(const Storm::Vector3 &dimension);
		
		// Tetrahedron formed by its 4 vertexes
		static float computeTetrahedronVolume(const Storm::Vector3(&vertexes)[4]);

		// Tetrahedron formed by 3 vertexes. The 4th vertex is implicitly considered to be 0 (Object space)
		static float computeTetrahedronVolume(const Storm::Vector3(&vertexes)[3]);
	};
}
