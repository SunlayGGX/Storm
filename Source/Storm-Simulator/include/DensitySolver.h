#pragma once

#include "NonInstanciable.h"
#include "ParticleIdentifier.h"


namespace Storm
{
	class DensitySolver : private Storm::NonInstanciable
	{
	public:
		static float computeDensityPCISPH(float particleMass, const std::vector<Storm::ParticleIdentifier> &particleNeighborhood);
	};
}