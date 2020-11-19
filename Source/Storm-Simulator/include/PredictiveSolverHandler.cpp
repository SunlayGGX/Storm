#include "PredictiveSolverHandler.h"

#include "UIField.h"
#include "UIFieldContainer.h"

#include "ParticleSystem.h"

#include "RunnerHelper.h"


#define STORM_AVERAGE_FIELD_NAME "Average error"


Storm::PredictiveSolverHandler::PredictiveSolverHandler(const wchar_t*const(&solversNames)[Storm::PredictiveSolverHandler::k_maxSolverCount]) :
	_solverCount{ 0 },
	_averageError{ 0.f },
	_uiFields{ std::make_unique<Storm::UIFieldContainer>() },
	_logNoOverloadIter{ 0 }
{
	assert(solverCount <= Storm::PredictiveSolverHandler::k_maxSolverCount && "We request too much solvers!");

	(*_uiFields)
		.bindField(STORM_AVERAGE_FIELD_NAME, _averageError)
		;

	for (; _solverCount < Storm::PredictiveSolverHandler::k_maxSolverCount; ++_solverCount)
	{
		const wchar_t* solverName = solversNames[_solverCount];
		if (solverName != nullptr)
		{
			unsigned int &currentPredictionIter = _solverPredictionIter[_solverCount];
			currentPredictionIter = 0;
			_solverNames[_solverCount] = solverName;
			_uiFields->bindFieldW(solverName, currentPredictionIter);
		}
		else
		{
			return;
		}
	}
}

Storm::PredictiveSolverHandler::~PredictiveSolverHandler() = default;

void Storm::PredictiveSolverHandler::updateCurrentPredictionIter(unsigned int newPredictionIter, const unsigned int expectedMaxPredictionIter, const float densityError, const float maxDensityError, const std::size_t solverIndex)
{
	if (densityError > maxDensityError && newPredictionIter > expectedMaxPredictionIter && (_logNoOverloadIter++ % 10 == 0))
	{
		LOG_DEBUG_WARNING <<
			"Max prediction loop watchdog hit without being able to go under the max density error allowed...\n"
			"We'll leave the prediction loop with an average density of " << densityError;
	}

	assert(solverIndex < _solverCount && "Requesting a solver info data that doesn't exist!");
	unsigned int &currentPredictionIter = _solverPredictionIter[solverIndex];
	if (currentPredictionIter != newPredictionIter)
	{
		currentPredictionIter = newPredictionIter;
		_uiFields->pushFieldW(_solverNames[solverIndex]);
	}

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
