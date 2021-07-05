#pragma once

#include "SPHBaseSolver.h"
#include "PredictiveSolverHandler.h"
#include "SPHSolverPrivateLogic.h"


namespace Storm
{
	struct DFSPHSolverData;
	struct SceneFluidCustomDFSPHConfig;

	class DFSPHSolverModified :
		public Storm::ISPHBaseSolver,
		private Storm::PredictiveSolverHandler,
		private Storm::SPHSolverPrivateLogic
	{
	private:
		using DFSPHSolverDataArray = std::vector<Storm::DFSPHSolverData>;

	public:
		DFSPHSolverModified(const float k_kernelLength, const Storm::ParticleSystemContainer &particleSystemsMap);
		~DFSPHSolverModified();

	public:
		void execute(const Storm::IterationParameter &iterationParameter) final override;
		void removeRawEndData(const unsigned int pSystemId, std::size_t toRemoveCount) final override;

	private:
		void onSubIterationStart(const Storm::ParticleSystemContainer &particleSystems);

	public:
		void setEnableThresholdDensity(bool enable);
		void setNeighborThresholdDensity(std::size_t neighborCount);

	private:
		void divergenceSolve(const Storm::IterationParameter &iterationParameter, unsigned int &outIteration, float &outAverageError);
		void pressureSolve(const Storm::IterationParameter &iterationParameter, unsigned int &outIteration, float &outAverageError);
		void computeDFSPHFactor(const Storm::IterationParameter &iterationParameter, Storm::FluidParticleSystem &fluidPSystem, Storm::DFSPHSolverModified::DFSPHSolverDataArray &pSystemData, const double kMultiplicationCoeff);
		void computeDensityAdv(const Storm::IterationParameter &iterationParameter, Storm::FluidParticleSystem &fluidPSystem, const Storm::DFSPHSolverModified::DFSPHSolverDataArray* currentSystemData, Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex);
		void computeDensityChange(const Storm::IterationParameter &iterationParameter, Storm::FluidParticleSystem &fluidPSystem, const Storm::DFSPHSolverModified::DFSPHSolverDataArray* currentSystemData, Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex);

	private:
		std::map<unsigned int, Storm::DFSPHSolverModified::DFSPHSolverDataArray> _data;
		double _totalParticleCountDb;
		bool _enableDivergenceSolve;
		bool _enableThresholdDensity;
		std::size_t _neighborThresholdDensity;
	};
}