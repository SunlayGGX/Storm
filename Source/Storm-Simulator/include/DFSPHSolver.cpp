#include "DFSPHSolver.h"

#include "ParticleSystem.h"

#include "ThrowException.h"


Storm::DFSPHSolver::DFSPHSolver(const float k_kernelLength, const Storm::ParticleSystemContainer &particleSystemsMap)
{
	std::size_t totalParticleCount = 0;

	for (const auto &particleSystemPair : particleSystemsMap)
	{
		const Storm::ParticleSystem &pSystem = *particleSystemPair.second;
		if (pSystem.isFluids())
		{
			totalParticleCount += particleSystemPair.second->getParticleCount();
		}
	}

	_totalParticleCountFl = static_cast<float>(totalParticleCount);
}

void Storm::DFSPHSolver::execute(Storm::ParticleSystemContainer &particleSystems, const float kernelLength, const float k_deltaTime)
{
	STORM_NOT_IMPLEMENTED;
}
