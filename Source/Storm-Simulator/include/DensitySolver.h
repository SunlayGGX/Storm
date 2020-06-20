#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	struct NeighborParticleInfo;

	class DensitySolver : private Storm::NonInstanciable
	{
	public:
		static float computeDensityPCISPH(float particleMass, const std::vector<Storm::NeighborParticleInfo> &particleNeighborhood);
	};
}
