#pragma once

#include "SPHBaseSolver.h"


namespace Storm
{
	class DFSPHSolver : public Storm::ISPHBaseSolver
	{
	public:
		DFSPHSolver();

	public:
		void execute(Storm::ParticleSystemContainer &particleSystems, const float kernelLength, const float k_deltaTime) final override;
	};
}