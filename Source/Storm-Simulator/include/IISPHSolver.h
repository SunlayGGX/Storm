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
		void execute(Storm::ParticleSystemContainer &particleSystems, const float k_kernelLength, const float k_deltaTime) final override;

	private:
		std::map<unsigned int, std::vector<Storm::IISPHSolverData>> _data;
		std::size_t _particleCount;
	};
}