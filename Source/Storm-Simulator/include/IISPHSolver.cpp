#include "IISPHSolver.h"

#include "IISPHSolverData.h"

#include "ParticleSystem.h"

#include "ThrowException.h"

#define STORM_HIJACKED_TYPE Storm::IISPHSolverData
#	include "VectHijack.h"
#undef STORM_HIJACKED_TYPE


Storm::IISPHSolver::IISPHSolver(const float k_kernelLength, const Storm::ParticleSystemContainer &particleSystemsMap) :
	_totalParticleCount{ 0 }
{
	for (const auto &particleSystemPair : particleSystemsMap)
	{
		const Storm::ParticleSystem &currentPSystem = *particleSystemPair.second;
		if (currentPSystem.isFluids())
		{
			const Storm::VectorHijacker currentPSystemPCount{ currentPSystem.getParticleCount() };

			std::vector<Storm::IISPHSolverData> &currentPSystemData = _data[particleSystemPair.first];
			currentPSystemData.reserve(currentPSystemPCount._newSize);
			Storm::setNumUninitialized_hijack(currentPSystemData, currentPSystemPCount);

			_totalParticleCount += currentPSystemPCount._newSize;
		}
	}
}

void Storm::IISPHSolver::execute(Storm::ParticleSystemContainer &particleSystems, const float k_kernelLength, const float k_deltaTime)
{
	STORM_NOT_IMPLEMENTED;

	// Note :
	// Even if some part of the algorithm is exactly the same as inside other solvers, I did not factorize on purpose (I did, but reverted immediately because it was a really bad idea) !
	// The reason is that the algorithm piece works for this solver. If a bug arise, then it could be because of this solver algorithm and don't have anything to do with other solvers algorithm,
	// therefore trying to fix the parent factorized method is not the right solution since it would risk to jeopardize all other solvers.
	//
	// Yes I know it is hard to maintain with all those copy-pasted piece of code, but it would be harder to improve/develop a specific solver where all modifications are shared and could break other solvers we didn't test (since I don't have any QA and don't have time to test every solvers myself, it is preferable to keep copy pasted code).
	// Therefore, if you detect a bug in any solvers, and think the bug would impact other solvers, check them manually one by one and fix the issue locally.

}
