#include "PredictiveSolverHandler.h"

#include "UIField.h"
#include "UIFieldContainer.h"

#include "ParticleSystem.h"

#include "RunnerHelper.h"


Storm::PredictiveSolverHandler::PredictiveSolverHandler(const Storm::PredictiveSolverHandler::SolversNames &solversIterationNames, const Storm::PredictiveSolverHandler::SolversNames &solversErrorsNames) :
	_solverCount{ 0 },
	_uiFields{ std::make_unique<Storm::UIFieldContainer>() },
	_logNoOverloadIter{ 0 }
{
	for (; _solverCount < Storm::PredictiveSolverHandler::k_maxSolverCount; ++_solverCount)
	{
		const wchar_t* solverIterationName = solversIterationNames[_solverCount];
		const wchar_t* solverErrorName = solversErrorsNames[_solverCount];
		if (solverIterationName != nullptr)
		{
			assert(solverErrorName != nullptr && "1 iteration value should be bound to 1 error!");

			unsigned int &currentPredictionIter = _solverPredictionIter[_solverCount];
			currentPredictionIter = 0;
			_solverIterationNames[_solverCount] = solverIterationName;

			float &currentAverageError = _averageErrors[_solverCount];
			currentAverageError = 0.f;
			_averageErrorsNames[_solverCount] = solverErrorName;

			(*_uiFields)
				.bindFieldW(solverIterationName, currentPredictionIter)
				.bindFieldW(solverErrorName, currentAverageError);
		}
		else
		{
			return;
		}
	}
}

Storm::PredictiveSolverHandler::~PredictiveSolverHandler() = default;

void Storm::PredictiveSolverHandler::updateCurrentPredictionIter(unsigned int newPredictionIter, const unsigned int expectedMaxPredictionIter, const float error, const float maxError, const std::size_t solverIndex)
{
	if (error > maxError && newPredictionIter > expectedMaxPredictionIter && (_logNoOverloadIter++ % 10 == 0))
	{
		LOG_DEBUG_WARNING <<
			"Max prediction loop watchdog hit without being able to go under the max error allowed for the solver " << solverIndex << "...\n"
			"We'll leave the prediction loop with an average error of " << error;
	}

	assert(solverIndex < _solverCount && "Requesting a solver info data that doesn't exist!");

	unsigned int &currentPredictionIter = _solverPredictionIter[solverIndex];
	if (currentPredictionIter != newPredictionIter)
	{
		currentPredictionIter = newPredictionIter;
		_uiFields->pushFieldW(_solverIterationNames[solverIndex]);
	}

	float &currentAverageError = _averageErrors[solverIndex];
	if (currentAverageError != error)
	{
		currentAverageError = error;
		_uiFields->pushFieldW(_averageErrorsNames[solverIndex]);
	}
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

void Storm::PredictiveSolverHandler::transfertEndDataToSystems(Storm::ParticleSystemContainer &particleSystems, const Storm::IterationParameter &iterationParameter, void* data, FluidTransfertCallback fluidTransfertCallback, const bool rbViscoTransfered /*= true*/)
{
	for (auto &particleSystemPair : particleSystems)
	{
		Storm::ParticleSystem &particleSystem = *particleSystemPair.second;
		if (particleSystem.isFluids())
		{
			fluidTransfertCallback(data, particleSystemPair.first, reinterpret_cast<Storm::FluidParticleSystem &>(particleSystem), iterationParameter);
		}
		else if (!particleSystem.isStatic())
		{
			if (rbViscoTransfered)
			{
				Storm::runParallel(particleSystem.getForces(), [&particleSystem](Storm::Vector3 &forces, const std::size_t currentPIndex)
				{
					forces = particleSystem.getTemporaryViscosityForces()[currentPIndex] + particleSystem.getTemporaryPressureForces()[currentPIndex];
				});
			}
			else
			{
				const std::vector<Storm::Vector3> &pressureForces = particleSystem.getTemporaryPressureForces();
				std::vector<Storm::Vector3> &forces = particleSystem.getForces();

				std::copy(std::execution::par, std::begin(pressureForces), std::end(pressureForces), std::begin(forces));
			}
		}
	}
}
