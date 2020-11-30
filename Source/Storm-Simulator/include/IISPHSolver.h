#pragma once

#include "SPHBaseSolver.h"
#include "PredictiveSolverHandler.h"


namespace Storm
{
	struct IISPHSolverData;

	class IISPHSolver :
		public Storm::ISPHBaseSolver,
		private Storm::PredictiveSolverHandler
	{
	public:
		IISPHSolver(const float k_kernelLength, const Storm::ParticleSystemContainer &particleSystemsMap);

	public:
		void execute(const Storm::IterationParameter &iterationParameter) final override;

	private:
		std::map<unsigned int, std::vector<Storm::IISPHSolverData>> _data;
		float _totalParticleCountFl;
	};
}