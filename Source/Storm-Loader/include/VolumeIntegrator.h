#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	class AssetCacheData;

	class VolumeIntegrator : private Storm::NonInstanciable
	{
	public:
		static float computeSphereVolume(const float radius);
		static float computeCubeVolume(const Storm::Vector3 &dimension);
		static float computePyramidVolume(const Storm::Vector3 &dimension);

		static float computeTriangleMeshVolume(const Storm::AssetCacheData &mesh);
	};
}
