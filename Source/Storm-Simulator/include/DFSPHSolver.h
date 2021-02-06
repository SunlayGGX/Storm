#pragma once

#include "SPHBaseSolver.h"
#include "PredictiveSolverHandler.h"


namespace Storm
{
	struct DFSPHSolverData;
	struct SceneFluidCustomDFSPHConfig;

	class DFSPHSolver :
		public Storm::ISPHBaseSolver,
		private Storm::PredictiveSolverHandler
	{
	private:
		using DFSPHSolverDataArray = std::vector<Storm::DFSPHSolverData>;

	public:
		DFSPHSolver(const float k_kernelLength, const Storm::ParticleSystemContainer &particleSystemsMap);
		~DFSPHSolver();

	public:
		void execute(const Storm::IterationParameter &iterationParameter) final override;

	private:
		void divergenceSolve(const Storm::IterationParameter &iterationParameter, unsigned int &outIteration, float &outAverageError);
		void pressureSolve(const Storm::IterationParameter &iterationParameter, unsigned int &outIteration, float &outAverageError);
		void computeDFSPHFactor(const Storm::IterationParameter &iterationParameter, Storm::FluidParticleSystem &fluidPSystem, Storm::DFSPHSolver::DFSPHSolverDataArray &pSystemData, const double kMultiplicationCoeff);
		void computeDensityAdv(const Storm::IterationParameter &iterationParameter, Storm::FluidParticleSystem &fluidPSystem, const Storm::DFSPHSolver::DFSPHSolverDataArray* currentSystemData, Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex);
		void computeDensityChange(const Storm::IterationParameter &iterationParameter, Storm::FluidParticleSystem &fluidPSystem, const Storm::DFSPHSolver::DFSPHSolverDataArray* currentSystemData, Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex, const Storm::SceneFluidCustomDFSPHConfig &dfsphFluidConfig);

	private:
		std::map<unsigned int, Storm::DFSPHSolver::DFSPHSolverDataArray> _data;
		float _totalParticleCountFl;
		bool _enableDivergenceSolve;
	};
}