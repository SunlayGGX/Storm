#pragma once

#include "SPHBaseSolver.h"


namespace Storm
{
	class IISPHSolver : public Storm::ISPHBaseSolver
	{
	public:
		IISPHSolver();

	public:
		void execute(Storm::ParticleSystemContainer &particleSystems, const float kernelLength, const float k_deltaTime) final override;
	};
}