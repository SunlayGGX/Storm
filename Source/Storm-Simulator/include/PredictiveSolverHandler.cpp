#include "PredictiveSolverHandler.h"

#include "UIField.h"
#include "UIFieldContainer.h"

#include "FluidParticleSystem.h"
#include "RigidBodyParticleSystem.h"

#include "FluidData.h"
#include "GeneralSimulationData.h"

#include "Kernel.h"

#include "RunnerHelper.h"


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

void Storm::PredictiveSolverHandler::computeNonPressureForce(Storm::ParticleSystemContainer &pSystemMap, const float k_kernelLength, const Storm::GeneralSimulationData &generalConfig, const Storm::FluidData &fluidConfigData, const std::function<void*(unsigned int)> &getterDataFunc, void(*initDataFunc)(void*, const std::size_t, const Storm::FluidParticleSystem &))
{
	const float k_kernelLengthSquared = k_kernelLength * k_kernelLength;

	const Storm::GradKernelMethodDelegate gradKernel = Storm::retrieveGradKernelMethod(generalConfig._kernelMode);

	for (auto &particleSystemPair : pSystemMap)
	{
		Storm::ParticleSystem &currentParticleSystem = *particleSystemPair.second;
		if (currentParticleSystem.isFluids())
		{
			Storm::FluidParticleSystem &fluidParticleSystem = static_cast<Storm::FluidParticleSystem &>(currentParticleSystem);

			const float density0 = fluidParticleSystem.getRestDensity();
			const float density0Squared = density0 * density0;
			const std::vector<Storm::ParticleNeighborhoodArray> &neighborhoodArrays = currentParticleSystem.getNeighborhoodArrays();
			const std::vector<float> &masses = fluidParticleSystem.getMasses();
			const std::vector<float> &densities = fluidParticleSystem.getDensities();
			std::vector<Storm::Vector3> &temporaryPViscoForces = fluidParticleSystem.getTemporaryViscosityForces();

			const std::vector<Storm::Vector3> &positions = fluidParticleSystem.getPositions();
			const std::vector<Storm::Vector3> &velocities = fluidParticleSystem.getVelocity();

			const float viscoPrecoeff = 0.01f * k_kernelLengthSquared;

			void* dataField = getterDataFunc(particleSystemPair.first);

			Storm::runParallel(fluidParticleSystem.getForces(), [&](Storm::Vector3 &currentPForce, const std::size_t currentPIndex)
			{
				const float currentPMass = masses[currentPIndex];
				const float currentPDensity = densities[currentPIndex];
				const Storm::Vector3 &vi = velocities[currentPIndex];

				const float restMassDensity = currentPMass * density0;

				const float viscoPrecoeff = 0.01f * k_kernelLengthSquared;

				Storm::Vector3 totalViscosityForceOnParticle = Storm::Vector3::Zero();

				const Storm::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoodArrays[currentPIndex];
				for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
				{
					const Storm::Vector3 gradKernel_NablaWij = gradKernel(k_kernelLength, neighbor._positionDifferenceVector, neighbor._vectToParticleNorm);

					const Storm::Vector3 vij = vi - neighbor._containingParticleSystem->getVelocity()[neighbor._particleIndex];

					const float vijDotXij = vij.dot(neighbor._positionDifferenceVector);
					const float viscoGlobalCoeff = currentPMass * 10.f * vijDotXij / (neighbor._vectToParticleSquaredNorm + viscoPrecoeff);

					Storm::Vector3 pressureComponent;
					Storm::Vector3 viscosityComponent;

					if (neighbor._isFluidParticle)
					{
						const Storm::FluidParticleSystem* neighborPSystemAsFluid = static_cast<Storm::FluidParticleSystem*>(neighbor._containingParticleSystem);
						const float neighborDensity0 = neighborPSystemAsFluid->getRestDensity();
						const float neighborMass = neighborPSystemAsFluid->getMasses()[neighbor._particleIndex];
						const float neighborRawDensity = neighborPSystemAsFluid->getDensities()[neighbor._particleIndex];
						const float neighborDensity = neighborRawDensity * density0 / neighborDensity0;
						const float neighborVolume = neighborPSystemAsFluid->getParticleVolume();

						// Viscosity
						viscosityComponent = (viscoGlobalCoeff * fluidConfigData._dynamicViscosity * neighborMass / neighborRawDensity) * gradKernel_NablaWij;
					}
					else
					{
						const Storm::RigidBodyParticleSystem* neighborPSystemAsBoundary = static_cast<Storm::RigidBodyParticleSystem*>(neighbor._containingParticleSystem);

						const float neighborVolume = neighborPSystemAsBoundary->getVolumes()[neighbor._particleIndex];
						const float rbViscosity = neighborPSystemAsBoundary->getViscosity();

						// Viscosity
						if (rbViscosity > 0.f)
						{
							viscosityComponent = (viscoGlobalCoeff * rbViscosity * neighborVolume * density0 / currentPDensity) * gradKernel_NablaWij;
						}
						else
						{
							viscosityComponent = Storm::Vector3::Zero();
						}

						// Mirror the force on the boundary solid following the 3rd newton law
						if (!neighborPSystemAsBoundary->isStatic())
						{
							Storm::Vector3 &boundaryNeighborForce = neighbor._containingParticleSystem->getForces()[neighbor._particleIndex];
							Storm::Vector3 &boundaryNeighborTmpPressureForce = neighbor._containingParticleSystem->getTemporaryPressureForces()[neighbor._particleIndex];
							Storm::Vector3 &boundaryNeighborTmpViscosityForce = neighbor._containingParticleSystem->getTemporaryViscosityForces()[neighbor._particleIndex];

							const Storm::Vector3 sumForces = pressureComponent + viscosityComponent;

							std::lock_guard<std::mutex> lock{ neighbor._containingParticleSystem->_mutex };
							boundaryNeighborForce -= sumForces;
							boundaryNeighborTmpViscosityForce -= viscosityComponent;
						}
					}

					totalViscosityForceOnParticle += viscosityComponent;
				}

				currentPForce += totalViscosityForceOnParticle;

				temporaryPViscoForces[currentPIndex] = totalViscosityForceOnParticle;

				// We should also initialize the data field now (avoid to restart the threads).
				initDataFunc(dataField, currentPIndex, fluidParticleSystem);
			});
		}
	}
}
