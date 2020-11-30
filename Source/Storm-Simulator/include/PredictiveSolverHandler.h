#pragma once

#include "ParticleSystemContainer.h"


namespace Storm
{
	class UIFieldContainer;
	class FluidParticleSystem;
	struct IterationParameter;

	class PredictiveSolverHandler
	{
	public:
		enum : std::size_t { k_maxSolverCount = 2 };

		using SolversNames = const wchar_t*const(&)[Storm::PredictiveSolverHandler::k_maxSolverCount];

	protected:
		PredictiveSolverHandler(const Storm::PredictiveSolverHandler::SolversNames &solversIterationNames, const Storm::PredictiveSolverHandler::SolversNames &solversErrorsNames);
		~PredictiveSolverHandler();

	protected:
		void updateCurrentPredictionIter(unsigned int newPredictionIter, const unsigned int expectedMaxPredictionIter, const float error, const float maxError, const std::size_t solverIndex);

	protected:
		void initializePredictionIteration(Storm::ParticleSystemContainer &particleSystems, float &averageDensityError);
		void transfertEndDataToSystems(Storm::ParticleSystemContainer &particleSystems, const Storm::IterationParameter &iterationParameter, void* data, void(*fluidTransfertCallback)(void*, const unsigned int, Storm::FluidParticleSystem &, const Storm::IterationParameter &));

	private:
		unsigned int _solverPredictionIter[Storm::PredictiveSolverHandler::k_maxSolverCount];
		std::wstring_view _solverIterationNames[Storm::PredictiveSolverHandler::k_maxSolverCount];

		float _averageErrors[Storm::PredictiveSolverHandler::k_maxSolverCount];
		std::wstring_view _averageErrorsNames[Storm::PredictiveSolverHandler::k_maxSolverCount];

		std::unique_ptr<Storm::UIFieldContainer> _uiFields;

		std::size_t _solverCount;

		uint8_t _logNoOverloadIter;
	};
}
