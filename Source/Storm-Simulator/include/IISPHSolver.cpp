#include "IISPHSolver.h"

#include "IISPHSolverData.h"

#include "ParticleSystem.h"

#include "ThrowException.h"

#define STORM_HIJACKED_TYPE Storm::IISPHSolverData
#	include "VectHijack.h"
#undef STORM_HIJACKED_TYPE


Storm::IISPHSolver::IISPHSolver(const float k_kernelLength, const Storm::ParticleSystemContainer &particleSystemsMap) :
	_particleCount{ 0 }
{
	STORM_NOT_IMPLEMENTED;
	for (const auto &particleSystemPair : particleSystemsMap)
	{
		const Storm::ParticleSystem &currentPSystem = *particleSystemPair.second;
		if (currentPSystem.isFluids())
		{
			const Storm::VectorHijacker currentPSystemPCount{ currentPSystem.getParticleCount() };

			std::vector<Storm::IISPHSolverData> &data = _datas[particleSystemPair.first];
			data.reserve(currentPSystemPCount._newSize);
			Storm::setNumUninitialized_hijack(data, currentPSystemPCount);

			_particleCount += currentPSystemPCount._newSize;
		}
	}
}

void Storm::IISPHSolver::execute(Storm::ParticleSystemContainer &particleSystems, const float kernelLength, const float k_deltaTime)
{
	STORM_NOT_IMPLEMENTED;
}
