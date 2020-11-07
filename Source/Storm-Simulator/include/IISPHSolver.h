#pragma once

#include "SPHBaseSolver.h"


namespace Storm
{
	struct IISPHSolverData;

	class IISPHSolver : public Storm::ISPHBaseSolver
	{
	public:
		IISPHSolver();

	public:
		void execute(Storm::ParticleSystemContainer &particleSystems, const float kernelLength, const float k_deltaTime) final override;

	private:
		std::map<unsigned int, std::vector<Storm::IISPHSolverData>> _data;
	};
}