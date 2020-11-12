#pragma once

#include "ParticleSystemContainer.h"


namespace Storm
{
	class UIFieldContainer;
	class FluidParticleSystem;

	class PredictiveSolverHandler
	{
	protected:
		PredictiveSolverHandler();
		~PredictiveSolverHandler();

	protected:
		void updateCurrentPredictionIter(unsigned int newPredictionIter, const unsigned int expectedMaxPredictionIter, const float densityError, const float maxDensityError);

	protected:
		void transfertEndDataToSystems(Storm::ParticleSystemContainer &particleSystems, void* data, void(*fluidTransfertCallback)(void*, const unsigned int, Storm::FluidParticleSystem &));

	private:
		unsigned int _currentPredictionIter;
		std::unique_ptr<Storm::UIFieldContainer> _uiFields;

		uint8_t _logNoOverloadIter;
	};
}
