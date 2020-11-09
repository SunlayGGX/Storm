#pragma once

#include "ParticleSystemContainer.h"


namespace Storm
{
	class UIFieldContainer;
	class FluidParticleSystem;
	struct GeneralSimulationData;
	struct FluidData;

	class PredictiveSolverHandler
	{
	protected:
		PredictiveSolverHandler();
		~PredictiveSolverHandler();

	protected:
		void updateCurrentPredictionIter(unsigned int newPredictionIter, const unsigned int expectedMaxPredictionIter, const float densityError, const float maxDensityError);

		void computeNonPressureForce(Storm::ParticleSystemContainer &pSystemMap, const float k_kernelLength, const Storm::GeneralSimulationData &generalConfig, const Storm::FluidData &fluidConfigData, const std::function<void*(unsigned int)> &getterDataFunc, void(*initDataFunc)(void*, const std::size_t, const Storm::FluidParticleSystem &));

	private:
		unsigned int _currentPredictionIter;
		std::unique_ptr<Storm::UIFieldContainer> _uiFields;

		uint8_t _logNoOverloadIter;
	};
}
