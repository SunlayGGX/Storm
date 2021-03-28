#include "SPHBaseSolver.h"

#include "SimulationMode.h"

#include "SolverCreationParameter.h"

#include "WCSPHSolver.h"
#include "PCISPHSolver.h"
#include "IISPHSolver.h"
#include "DFSPHSolver.h"
#include "DFSPHSolverModified.h"


std::unique_ptr<Storm::ISPHBaseSolver> Storm::instantiateSPHSolver(const Storm::SolverCreationParameter &creationParameter)
{
	switch (creationParameter._simulationMode)
	{
	case Storm::SimulationMode::WCSPH: return std::make_unique<Storm::WCSPHSolver>();
	case Storm::SimulationMode::PCISPH: return std::make_unique<Storm::PCISPHSolver>(creationParameter._kernelLength, *creationParameter._particleSystems);
	case Storm::SimulationMode::IISPH: return std::make_unique<Storm::IISPHSolver>(creationParameter._kernelLength, *creationParameter._particleSystems);
	case Storm::SimulationMode::DFSPH: return std::make_unique<Storm::DFSPHSolver>(creationParameter._kernelLength, *creationParameter._particleSystems);
	case Storm::SimulationMode::DFSPHModified: return std::make_unique<Storm::DFSPHSolverModified>(creationParameter._kernelLength, *creationParameter._particleSystems);

	default:
		Storm::throwException<Storm::Exception>("Unknown simulation mode!");
	}
}
