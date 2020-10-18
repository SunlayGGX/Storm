#pragma once

#include "SPHBaseSolver.h"


namespace Storm
{
	class ParticleSystem;
	class ParticleSelector;

	class PCISPHSolver : public Storm::ISPHBaseSolver
	{
	public:
		PCISPHSolver(const float k_kernelLength);

	public:
		void execute(const std::map<unsigned int, std::unique_ptr<Storm::ParticleSystem>> &particleSystems, const float kernelLength, const float k_deltaTime) final override;

	private:
		float _kUniformStiffnessConstCoefficient;
	};
}
