#pragma once

#include "SPHBaseSolver.h"
#include "PredictiveSolverHandler.h"


namespace Storm
{
	class DFSPHSolver :
		public Storm::ISPHBaseSolver,
		private Storm::PredictiveSolverHandler
	{
	public:
		DFSPHSolver();

	public:
		void execute(Storm::ParticleSystemContainer &particleSystems, const float kernelLength, const float k_deltaTime) final override;
	};
}