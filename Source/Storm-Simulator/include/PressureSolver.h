#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	struct NeighborParticleInfo;

	class PressureSolver : private Storm::NonInstanciable
	{
	public:
		static Storm::Vector3 computeDensityPCISPH(float particleMass, const std::vector<Storm::NeighborParticleInfo> &particleNeighborhood);
	};
}
