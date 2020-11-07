#include "SPHBaseSolver.h"

#include "SimulationMode.h"

#include "ThrowException.h"

#include "WCSPHSolver.h"
#include "PCISPHSolver.h"
#include "IISPHSolver.h"
#include "DFSPHSolver.h"


std::unique_ptr<Storm::ISPHBaseSolver> Storm::instantiateSPHSolver(const Storm::SimulationMode simulationMode, const float k_kernelLength, const Storm::ParticleSystemContainer &particleSystems)
{
	switch (simulationMode)
	{
	case Storm::SimulationMode::WCSPH: return std::make_unique<Storm::WCSPHSolver>();
	case Storm::SimulationMode::PCISPH: return std::make_unique<Storm::PCISPHSolver>(k_kernelLength, particleSystems);
	case Storm::SimulationMode::IISPH: return std::make_unique<Storm::IISPHSolver>(k_kernelLength, particleSystems);
	case Storm::SimulationMode::DFSPH: return std::make_unique<Storm::DFSPHSolver>();

	default:
		Storm::throwException<std::exception>("Unknown simulation mode!");
	}
}
