#pragma once

#include "SPHBaseSolver.h"


namespace Storm
{
	struct PCISPHSolverData;
	class UIFieldContainer;

	class PCISPHSolver : public Storm::ISPHBaseSolver
	{
	public:
		PCISPHSolver(const float k_kernelLength, const Storm::ParticleSystemContainer &particleSystemsMap);

	public:
		void execute(Storm::ParticleSystemContainer &particleSystems, const float kernelLength, const float k_deltaTime) final override;

	private:
		void setCurrentPredictionIter(unsigned int newValue);

	private:
		float _kUniformStiffnessConstCoefficient;
		std::map<unsigned int, std::vector<Storm::PCISPHSolverData>> _data;
		std::size_t _totalParticleCount;

		unsigned int _currentPredictionIter;
		std::unique_ptr<Storm::UIFieldContainer> _uiFields;

		uint8_t _logNoOverloadIter;
	};
}
