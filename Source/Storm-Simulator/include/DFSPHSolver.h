#pragma once

#include "SPHBaseSolver.h"
#include "PredictiveSolverHandler.h"


namespace Storm
{
	struct DFSPHSolverData;

	class DFSPHSolver :
		public Storm::ISPHBaseSolver,
		private Storm::PredictiveSolverHandler
	{
	public:
		DFSPHSolver(const float k_kernelLength, const Storm::ParticleSystemContainer &particleSystemsMap);
		~DFSPHSolver();

	public:
		void execute(Storm::ParticleSystemContainer &particleSystems, const float kernelLength, const float k_deltaTime) final override;

	private:
		std::map<unsigned int, std::vector<Storm::DFSPHSolverData>> _data;
		float _totalParticleCountFl;
	};
}