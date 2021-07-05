#pragma once

#include "SPHBaseSolver.h"
#include "SPHSolverPrivateLogic.h"


namespace Storm
{
	class WCSPHSolver :
		public Storm::ISPHBaseSolver,
		private Storm::SPHSolverPrivateLogic
	{
	public:
		void execute(const Storm::IterationParameter &iterationParameter) final override;
		void removeRawEndData(const unsigned int pSystemId, std::size_t toRemoveCount) final override;
	};
}
