#include "DensitySolver.h"

#include "SingletonHolder.h"
#include "SimulatorManager.h"

#include "Poly6Kernel.h"

#include "NeighborParticleInfo.h"


float Storm::DensitySolver::computeDensityPCISPH(float particleMass, const std::vector<Storm::NeighborParticleInfo> &particleNeighborhood)
{
	float resultDensity = 0.f;

	const Storm::SimulatorManager &simulMgr = Storm::SimulatorManager::instance();

	Storm::Poly6Kernel poly6Kernel{ simulMgr.getKernelLength() };

	for (const Storm::NeighborParticleInfo &neighborhoodParticleIdent : particleNeighborhood)
	{
		resultDensity += particleMass * poly6Kernel(neighborhoodParticleIdent._vectToParticleSquaredNorm);

		// TODO : Compute corrected density using Shepard filter (Xavier Chermain Memoire : equation 2.8) to handle boundary handling.
	}

	return resultDensity;
}
