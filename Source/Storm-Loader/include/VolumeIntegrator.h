#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	class AssetCacheData;

	class VolumeIntegrator : private Storm::NonInstanciable
	{
	public:
		static float computeSphereVolume(const Storm::Vector3 &dimension);
		static float computeCubeVolume(const Storm::Vector3 &dimension);
		static float computeTetrahedronVolume(const Storm::Vector3(&vertexes)[4]);

		static float computeTriangleMeshVolume(const Storm::AssetCacheData &mesh);
	};
}
