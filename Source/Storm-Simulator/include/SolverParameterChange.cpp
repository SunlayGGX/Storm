#include "SolverParameterChange.h"

#include "DFSPHSolver.h"

#include "ThreadingSafety.h"


namespace
{
	template<class Type, class ... Args, class Func>
	bool applyIfPossible(Storm::ISPHBaseSolver*const currentSolver, Func &func)
	{
		return
			applyIfPossible<Type>(currentSolver, func) ||
			applyIfPossible<Args...>(currentSolver, func);
	}

	template<class Type, class Func>
	bool applyIfPossible(Storm::ISPHBaseSolver*const currentSolver, Func &&func)
	{
		if (auto toType = dynamic_cast<Type*>(currentSolver); toType != nullptr)
		{
			func(*toType);
			return true;
		}

		return false;
	}

	void logChangeIgnored()
	{
		LOG_DEBUG_WARNING << "Solver changes ignored because we don't run the right solver to make the change";
	}
}


void Storm::SolverParameterChange::setEnableThresholdDensity_DFSPH(Storm::ISPHBaseSolver* currentSolver, bool enable)
{
	assert(Storm::isSimulationThread() && "Cannot change Sph solvers values outside the simulation thread!");

	if (applyIfPossible<Storm::DFSPHSolver>(currentSolver, [&enable](auto &solver)
	{
		solver.setEnableThresholdDensity(enable);
	}))
	{
		LOG_DEBUG << "Threshold density " << (enable ? "enabled" : "disabled");
	}
	else
	{
		logChangeIgnored();
	}
}

void Storm::SolverParameterChange::setNeighborThresholdDensity_DFSPH(Storm::ISPHBaseSolver* currentSolver, std::size_t neighborCount)
{
	assert(Storm::isSimulationThread() && "Cannot change Sph solvers values outside the simulation thread!");

	if (applyIfPossible<Storm::DFSPHSolver>(currentSolver, [&neighborCount](auto &solver)
	{
		solver.setNeighborThresholdDensity(neighborCount);
	}))
	{
		LOG_DEBUG << "Threshold density neighbor count changed to " << neighborCount;
	}
	else
	{
		logChangeIgnored();
	}
}
