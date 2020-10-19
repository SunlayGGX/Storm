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
		void execute(const Storm::ParticleSystemContainer &particleSystems, const float kernelLength, const float k_deltaTime) final override;

	private:
		float _kUniformStiffnessConstCoefficient;
	};
}
