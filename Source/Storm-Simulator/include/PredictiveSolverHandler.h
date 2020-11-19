#pragma once

#include "ParticleSystemContainer.h"


namespace Storm
{
	class UIFieldContainer;
	class FluidParticleSystem;

	class PredictiveSolverHandler
	{
	public:
		enum : std::size_t { k_maxSolverCount = 2 };

	protected:
		PredictiveSolverHandler(const wchar_t*const(&solversNames)[Storm::PredictiveSolverHandler::k_maxSolverCount]);
		~PredictiveSolverHandler();

	protected:
		void updateCurrentPredictionIter(unsigned int newPredictionIter, const unsigned int expectedMaxPredictionIter, const float densityError, const float maxDensityError, const std::size_t solverIndex);

	protected:
		void initializePredictionIteration(Storm::ParticleSystemContainer &particleSystems, float &averageDensityError);
		void transfertEndDataToSystems(Storm::ParticleSystemContainer &particleSystems, void* data, void(*fluidTransfertCallback)(void*, const unsigned int, Storm::FluidParticleSystem &));

	private:
		unsigned int _solverPredictionIter[Storm::PredictiveSolverHandler::k_maxSolverCount];
		std::wstring_view _solverNames[Storm::PredictiveSolverHandler::k_maxSolverCount];
		float _averageError;
		std::unique_ptr<Storm::UIFieldContainer> _uiFields;

		std::size_t _solverCount;

		uint8_t _logNoOverloadIter;
	};
}
