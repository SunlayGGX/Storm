#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	class ISPHBaseSolver;

	class SolverParameterChange : private Storm::NonInstanciable
	{
	public:
		static void setEnableThresholdDensity_DFSPH(Storm::ISPHBaseSolver* currentSolver, bool enable);
		static void setNeighborThresholdDensity_DFSPH(Storm::ISPHBaseSolver* currentSolver, std::size_t neighborCount);
	};
}
