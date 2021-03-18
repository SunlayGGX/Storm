#pragma once

#include "NonInstanciable.h"
#include "VolumeIntegrator.h"


namespace Storm
{
	class AssetCacheData;

	class AssetVolumeIntegrator : public Storm::VolumeIntegrator
	{
	public:
		static float computeTriangleMeshVolume(const Storm::AssetCacheData &mesh);
	};
}
