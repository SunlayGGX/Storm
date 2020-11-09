#include "PredictiveSolverHandler.h"

#include "UIField.h"
#include "UIFieldContainer.h"


#define STORM_PREDICTION_COUNT_FIELD_NAME "Prediction count"


Storm::PredictiveSolverHandler::PredictiveSolverHandler() :
	_currentPredictionIter{ 0 },
	_uiFields{ std::make_unique<Storm::UIFieldContainer>() },
	_logNoOverloadIter{ 0 }
{
	(*_uiFields)
		.bindField(STORM_PREDICTION_COUNT_FIELD_NAME, _currentPredictionIter)
		;
}

Storm::PredictiveSolverHandler::~PredictiveSolverHandler() = default;

void Storm::PredictiveSolverHandler::updateCurrentPredictionIter(unsigned int newPredictionIter, const unsigned int expectedMaxPredictionIter, const float densityError, const float maxDensityError)
{
	if (densityError > maxDensityError && newPredictionIter > expectedMaxPredictionIter && (_logNoOverloadIter++ % 10 == 0))
	{
		LOG_DEBUG_WARNING <<
			"Max prediction loop watchdog hit without being able to go under the max density error allowed...\n"
			"We'll leave the prediction loop with an average density of " << densityError;
	}

	if (_currentPredictionIter != newPredictionIter)
	{
		_currentPredictionIter = newPredictionIter;
		_uiFields->pushField(STORM_PREDICTION_COUNT_FIELD_NAME);
	}
}
