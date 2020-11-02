#pragma once

#include "SPHBaseSolver.h"


namespace Storm
{
	struct PCISPHSolverData;

	class PCISPHSolver : public Storm::ISPHBaseSolver
	{
	public:
		PCISPHSolver(const float k_kernelLength, const Storm::ParticleSystemContainer &particleSystemsMap);

	public:
		void execute(Storm::ParticleSystemContainer &particleSystems, const float kernelLength, const float k_deltaTime) final override;

	private:
		float _kUniformStiffnessConstCoefficient;
		std::map<unsigned int, std::vector<Storm::PCISPHSolverData>> _data;
		std::size_t _totalParticleCount;

		uint8_t _logNoOverloadIter;
	};
}
