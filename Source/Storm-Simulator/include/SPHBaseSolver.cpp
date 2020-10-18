#include "SPHBaseSolver.h"

#include "SimulationMode.h"

#include "ThrowException.h"

#include "WCSPHSolver.h"
#include "PCISPHSolver.h"


std::unique_ptr<Storm::ISPHBaseSolver> Storm::instantiateSPHSolver(const Storm::SimulationMode simulationMode)
{
	switch (simulationMode)
	{
	case Storm::SimulationMode::WCSPH: return std::make_unique<Storm::WCSPHSolver>();
	case Storm::SimulationMode::PCISPH: return std::make_unique<Storm::PCISPHSolver>();

	default:
		Storm::throwException<std::exception>("Unknown simulation mode!");
	}
}
