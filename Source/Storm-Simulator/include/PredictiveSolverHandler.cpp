#include "PredictiveSolverHandler.h"

#include "UIField.h"
#include "UIFieldContainer.h"

#include "ParticleSystem.h"

#include "RunnerHelper.h"


#define STORM_PREDICTION_COUNT_FIELD_NAME "Prediction count"
#define STORM_AVERAGE_FIELD_NAME "Average error"


Storm::PredictiveSolverHandler::PredictiveSolverHandler() :
	_currentPredictionIter{ 0 },
	_averageError{ 0.f },
	_uiFields{ std::make_unique<Storm::UIFieldContainer>() },
	_logNoOverloadIter{ 0 }
{
	(*_uiFields)
		.bindField(STORM_PREDICTION_COUNT_FIELD_NAME, _currentPredictionIter)
		.bindField(STORM_AVERAGE_FIELD_NAME, _averageError)
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

	updateField(*_uiFields, STORM_PREDICTION_COUNT_FIELD_NAME, _currentPredictionIter, newPredictionIter);
	updateField(*_uiFields, STORM_AVERAGE_FIELD_NAME, _averageError, densityError);
}

void Storm::PredictiveSolverHandler::initializePredictionIteration(Storm::ParticleSystemContainer &particleSystems, float &averageDensityError)
{
	for (auto &particleSystemPair : particleSystems)
	{
		Storm::ParticleSystem &particleSystem = *particleSystemPair.second;
		if (!particleSystem.isFluids() && !particleSystem.isStatic())
		{
			// For all dynamic rigid bodies, reset the temporary pressure force since we don't have temporary data for them.
			Storm::runParallel(particleSystem.getTemporaryPressureForces(), [](Storm::Vector3 &pressuresForces)
			{
				pressuresForces.setZero();
			});
		}
	}

	averageDensityError = 0.f;
}

void Storm::PredictiveSolverHandler::transfertEndDataToSystems(Storm::ParticleSystemContainer &particleSystems, void* data, void(*fluidTransfertCallback)(void*, const unsigned int, Storm::FluidParticleSystem &))
{
	for (auto &particleSystemPair : particleSystems)
	{
		Storm::ParticleSystem &particleSystem = *particleSystemPair.second;
		if (particleSystem.isFluids())
		{
			fluidTransfertCallback(data, particleSystemPair.first, reinterpret_cast<Storm::FluidParticleSystem &>(particleSystem));
		}
		else if (!particleSystem.isStatic())
		{
			Storm::runParallel(particleSystem.getForces(), [&particleSystem](Storm::Vector3 &forces, const std::size_t currentPIndex)
			{
				forces = particleSystem.getTemporaryViscosityForces()[currentPIndex] + particleSystem.getTemporaryPressureForces()[currentPIndex];
			});
		}
	}
}
