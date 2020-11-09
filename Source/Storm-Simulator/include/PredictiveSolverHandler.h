#pragma once


namespace Storm
{
	class UIFieldContainer;
	class PredictiveSolverHandler
	{
	protected:
		PredictiveSolverHandler();
		~PredictiveSolverHandler();

	protected:
		void updateCurrentPredictionIter(unsigned int newPredictionIter, const unsigned int expectedMaxPredictionIter, const float densityError, const float maxDensityError);

	private:
		unsigned int _currentPredictionIter;
		std::unique_ptr<Storm::UIFieldContainer> _uiFields;

		uint8_t _logNoOverloadIter;
	};
}
