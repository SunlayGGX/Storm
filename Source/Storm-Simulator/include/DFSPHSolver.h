#pragma once

#include "SPHBaseSolver.h"
#include "PredictiveSolverHandler.h"
#include "SPHSolverPrivateLogic.h"


namespace Storm
{
	struct DFSPHSolverData;
	struct SceneFluidCustomDFSPHConfig;
	struct SceneSimulationConfig;
	struct SceneFluidConfig;

	class DFSPHSolver :
		public Storm::ISPHBaseSolver,
		private Storm::PredictiveSolverHandler,
		private Storm::SPHSolverPrivateLogic
	{
	private:
		using DFSPHSolverDataArray = std::vector<Storm::DFSPHSolverData>;

	public:
		DFSPHSolver(const float k_kernelLength, const Storm::ParticleSystemContainer &particleSystemsMap);
		~DFSPHSolver();

	public:
		void execute(const Storm::IterationParameter &iterationParameter) final override;
		void removeRawEndData(const unsigned int pSystemId, std::size_t toRemoveCount) final override;

	public:
		void setEnableThresholdDensity(bool enable);
		void setUseRotationFix(bool useFix);
		void setNeighborThresholdDensity(std::size_t neighborCount);

	private:
		void initializeStepDensities(const Storm::IterationParameter &iterationParameter, const Storm::SceneSimulationConfig &sceneSimulationConfig, const Storm::SceneFluidCustomDFSPHConfig &sceneDFSPHSimulationConfig);
		void fullVelocityDivergenceSolve_Internal(const Storm::IterationParameter &iterationParameter, const Storm::SceneFluidConfig &scenefluidConfig, const Storm::SceneFluidCustomDFSPHConfig &sceneDFSPHSimulationConfig);
		void fullDensityInvariantSolve_Internal(const Storm::IterationParameter &iterationParameter, const Storm::SceneFluidCustomDFSPHConfig &sceneDFSPHSimulationConfig);
		void computeNonPressureForces_Internal(const Storm::IterationParameter &iterationParameter, const Storm::SceneSimulationConfig &sceneSimulationConfig, const Storm::SceneFluidConfig &fluidConfig);

	private:
		void divergenceSolve(const Storm::IterationParameter &iterationParameter, const Storm::SceneFluidCustomDFSPHConfig &sceneDFSPHSimulationConfig, unsigned int &outIteration, float &outAverageError);
		void pressureSolve(const Storm::IterationParameter &iterationParameter, const Storm::SceneFluidCustomDFSPHConfig &sceneDFSPHSimulationConfig, unsigned int &outIteration, float &outAverageError);
		void computeDFSPHFactor(const Storm::IterationParameter &iterationParameter, Storm::FluidParticleSystem &fluidPSystem, Storm::DFSPHSolver::DFSPHSolverDataArray &pSystemData, const double kMultiplicationCoeff);
		void computeDensityAdv(const Storm::IterationParameter &iterationParameter, Storm::FluidParticleSystem &fluidPSystem, const Storm::DFSPHSolver::DFSPHSolverDataArray* currentSystemData, Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex);
		void computeDensityChange(const Storm::IterationParameter &iterationParameter, Storm::FluidParticleSystem &fluidPSystem, const Storm::DFSPHSolver::DFSPHSolverDataArray* currentSystemData, Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex);

	private:
		std::map<unsigned int, Storm::DFSPHSolver::DFSPHSolverDataArray> _data;
		float _totalParticleCountFl;
		bool _enableDensitySolve;
		bool _enableThresholdDensity;
		bool _useRotationFix;
		std::size_t _neighborThresholdDensity;
	};
}
