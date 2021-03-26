#pragma once

#include "SPHBaseSolver.h"
#include "PredictiveSolverHandler.h"


namespace Storm
{
	struct PCISPHSolverData;

	class PCISPHSolver :
		public Storm::ISPHBaseSolver,
		private Storm::PredictiveSolverHandler
	{
	public:
		PCISPHSolver(const float k_kernelLength, const Storm::ParticleSystemContainer &particleSystemsMap);

	public:
		void execute(const Storm::IterationParameter &iterationParameter) final override;
		void removeRawEndData(const unsigned int pSystemId, std::size_t toRemoveCount) final override;

	private:
		float _kUniformStiffnessConstCoefficient;
		std::map<unsigned int, std::vector<Storm::PCISPHSolverData>> _data;
		std::size_t _totalParticleCount;
	};
}
