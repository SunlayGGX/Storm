#pragma once

#include "SPHBaseSolver.h"


namespace Storm
{
	class WCSPHSolver : public Storm::ISPHBaseSolver
	{
	public:
		void execute(const Storm::IterationParameter &iterationParameter) final override;
	};
}
