#pragma once

#include "NonInstanciable.h"
#include "NeighborParticleInfo.h"


namespace Storm
{
	class ViscositySolver : private Storm::NonInstanciable
	{
	public:
		static Storm::Vector3 computeViscosityForcePCISPH(float currentParticleMass, float currentParticleDensity, const Storm::Vector3 &currentParticleVelocity, const std::vector<Storm::NeighborParticleInfo> &particleNeighborhood);
	};
}
