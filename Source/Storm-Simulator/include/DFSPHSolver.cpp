#include "DFSPHSolver.h"

#include "ParticleSystem.h"

#include "DFSPHSolverData.h"

#include "ThrowException.h"

#define STORM_HIJACKED_TYPE Storm::DFSPHSolverData
#	include "VectHijack.h"
#undef STORM_HIJACKED_TYPE


namespace
{
	constexpr static const wchar_t* g_solverNames[Storm::PredictiveSolverHandler::k_maxSolverCount]
	{
		STORM_TEXT("Density solve iteration"),
		STORM_TEXT("Divergence solve iteration")
	};
}


Storm::DFSPHSolver::DFSPHSolver(const float k_kernelLength, const Storm::ParticleSystemContainer &particleSystemsMap) :
	Storm::PredictiveSolverHandler{ g_solverNames }
{
	std::size_t totalParticleCount = 0;

	for (const auto &particleSystemPair : particleSystemsMap)
	{
		const Storm::ParticleSystem &pSystem = *particleSystemPair.second;
		if (pSystem.isFluids())
		{
			const Storm::VectorHijacker currentPSystemPCount{ pSystem.getParticleCount() };

			std::vector<Storm::DFSPHSolverData> &currentPSystemData = _data[particleSystemPair.first];
			currentPSystemData.reserve(currentPSystemPCount._newSize);
			Storm::setNumUninitialized_hijack(currentPSystemData, currentPSystemPCount);

			totalParticleCount += currentPSystemPCount._newSize;
		}
	}

	_totalParticleCountFl = static_cast<float>(totalParticleCount);
}

Storm::DFSPHSolver::~DFSPHSolver() = default;

void Storm::DFSPHSolver::execute(Storm::ParticleSystemContainer &particleSystems, const float k_kernelLength, const float k_deltaTime)
{
	STORM_NOT_IMPLEMENTED;
}
