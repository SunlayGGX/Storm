#include "PCISPHSolver.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "GeneralSimulationData.h"
#include "FluidData.h"

#include "PCISPHSolverData.h"

#include "ParticleSystem.h"
#include "FluidParticleSystem.h"
#include "RigidBodyParticleSystem.h"

#include "Kernel.h"

#include "RunnerHelper.h"

#define STORM_HIJACKED_TYPE Storm::PCISPHSolverData
#	include "VectHijack.h"
#undef STORM_HIJACKED_TYPE


Storm::PCISPHSolver::PCISPHSolver(const float k_kernelLength, const Storm::ParticleSystemContainer &particleSystemsMap) :
	_totalParticleCount{ 0 }
{
	// Compute the uniform stiffness k_PCI coefficient
	Storm::Vector3d uniformNeighborStiffnessCoeffSum = Storm::Vector3d::Zero();
	double uniformNeighborStiffnessCoeffProd = 0.0;

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
	const Storm::GeneralSimulationData &generalSimulData = configMgr.getGeneralSimulationData();

	const float coordinateBegin = -k_kernelLength;
	const float particleDiameter = generalSimulData._particleRadius * 2.f;
	const unsigned int uniformComputationEndIter = std::max(static_cast<unsigned int>(std::ceilf(k_kernelLength / generalSimulData._particleRadius)) - 1, static_cast<unsigned int>(0));

	const auto k_gradMethod = Storm::retrieveGradKernelMethod(generalSimulData._kernelMode);

	Storm::Vector3 neighborUniformPos;

	const double k_templatePVolume = Storm::ParticleSystem::computeParticleDefaultVolume();
	const double k_templatePVolumeSquared = k_templatePVolume * k_templatePVolume;

	for (unsigned int xIter = 1; xIter < uniformComputationEndIter; ++xIter)
	{
		neighborUniformPos.x() = coordinateBegin + static_cast<float>(xIter) * particleDiameter;
		for (unsigned int yIter = 1; yIter < uniformComputationEndIter; ++yIter)
		{
			neighborUniformPos.y() = coordinateBegin + static_cast<float>(yIter) * particleDiameter;
			for (unsigned int zIter = 1; zIter < uniformComputationEndIter; ++zIter)
			{
				neighborUniformPos.z() = coordinateBegin + static_cast<float>(zIter) * particleDiameter;

				const float norm = neighborUniformPos.norm();
				if (norm < k_kernelLength && norm > 0.0000001f)
				{
					const Storm::Vector3 gradVal = k_gradMethod(k_kernelLength, neighborUniformPos, norm);

					// The values goes quickly really high, therefore k_templatePVolumeSquared is to reduce them to prevent going into the inaccurate values of the data type.
					uniformNeighborStiffnessCoeffSum += gradVal.cast<double>() * k_templatePVolumeSquared;
					uniformNeighborStiffnessCoeffProd += gradVal.squaredNorm() * k_templatePVolumeSquared;
				}
			}
		}
	}

	_kUniformStiffnessConstCoefficient = static_cast<float>(-0.5 / (uniformNeighborStiffnessCoeffSum.squaredNorm() + uniformNeighborStiffnessCoeffProd));

	// Initialize the velocity field
	for (const auto &particleSystemPair : particleSystemsMap)
	{
		const Storm::ParticleSystem &particleSystem = *particleSystemPair.second;
		if (particleSystem.isFluids())
		{
			std::vector<Storm::PCISPHSolverData> &dataField = _data[particleSystemPair.first];

			Storm::VectorHijacker hijacker{ particleSystem.getParticleCount() };
			dataField.reserve(hijacker._newSize);
			Storm::setNumUninitialized_hijack(dataField, hijacker);

			_totalParticleCount += hijacker._newSize;
		}
	}
}

void Storm::PCISPHSolver::execute(Storm::ParticleSystemContainer &particleSystems, const float k_kernelLength, const float k_deltaTime)
{
	STORM_NOT_IMPLEMENTED;

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
	const Storm::GeneralSimulationData &generalSimulData = configMgr.getGeneralSimulationData();
	const Storm::FluidData &fluidConfigData = configMgr.getFluidData();

	const float k_kernelZero = Storm::retrieveKernelZeroValue(generalSimulData._kernelMode);
	const Storm::RawKernelMethodDelegate rawKernel = Storm::retrieveRawKernelMethod(generalSimulData._kernelMode);
	const Storm::GradKernelMethodDelegate gradKernel = Storm::retrieveGradKernelMethod(generalSimulData._kernelMode);

	const float k_maxDensityError = generalSimulData._maxDensityError;
	unsigned int currentPredictionIter = 0;

	const float deltaTimeSquared = k_deltaTime * k_deltaTime;

	// First : compute stiffness constant coeff kPCI.
	const float k_templatePStiffnessCoeffK = _kUniformStiffnessConstCoefficient / deltaTimeSquared;

	float averageDensityError;

	// 2nd : Compute the base density
	for (auto &particleSystemPair : particleSystems)
	{
		Storm::ParticleSystem &currentParticleSystem = *particleSystemPair.second;
		if (currentParticleSystem.isFluids())
		{
			Storm::FluidParticleSystem &fluidParticleSystem = static_cast<Storm::FluidParticleSystem &>(currentParticleSystem);

			const float particleVolume = fluidParticleSystem.getParticleVolume();
			const float density0 = fluidParticleSystem.getRestDensity();
			std::vector<float> &pressures = fluidParticleSystem.getPressures();
			const std::vector<Storm::ParticleNeighborhoodArray> &neighborhoodArrays = fluidParticleSystem.getNeighborhoodArrays();

			Storm::runParallel(fluidParticleSystem.getDensities(), [&](float &currentPDensity, const std::size_t currentPIndex)
			{
				// Density
				currentPDensity = particleVolume * k_kernelZero;

				const Storm::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoodArrays[currentPIndex];
				for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
				{
					const float kernelValue_Wij = rawKernel(k_kernelLength, neighbor._vectToParticleNorm);
					float deltaDensity;
					if (neighbor._isFluidParticle)
					{
						deltaDensity = static_cast<Storm::FluidParticleSystem*>(neighbor._containingParticleSystem)->getParticleVolume() * kernelValue_Wij;
					}
					else
					{
						deltaDensity = static_cast<Storm::RigidBodyParticleSystem*>(neighbor._containingParticleSystem)->getVolumes()[neighbor._particleIndex] * kernelValue_Wij;
					}
					currentPDensity += deltaDensity;
				}

				// Volume * density is mass...
				currentPDensity *= density0;

				// Pressure
				float &currentPPressure = pressures[currentPIndex];
				if (currentPDensity < density0)
				{
					currentPDensity = density0;
					currentPPressure = 0.f;
				}
				else
				{
					currentPPressure = k_templatePStiffnessCoeffK * (density0 - currentPDensity);
				}
			});
		}
	}

	// 3rd : Compute the non pressure forces (viscosity)
	for (auto &particleSystemPair : particleSystems)
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
			const std::vector<Storm::Vector3> &velocities = fluidParticleSystem.getVelocity();
			std::vector<Storm::Vector3> &temporaryPViscoForce = fluidParticleSystem.getTemporaryViscosityForces();
			
			std::vector<Storm::PCISPHSolverData> &dataField = _data.find(particleSystemPair.first)->second;

			Storm::runParallel(fluidParticleSystem.getForces(), [&](Storm::Vector3 &currentPForce, const std::size_t currentPIndex)
			{
				const float currentPMass = masses[currentPIndex];
				const float currentPDensity = densities[currentPIndex];
				const Storm::Vector3 &vi = velocities[currentPIndex];

				const float restMassDensity = currentPMass * density0;

				const float viscoPrecoeff = 0.01f * k_kernelLength * k_kernelLength;

				Storm::Vector3 totalViscosityForceOnParticle = Storm::Vector3::Zero();

				const Storm::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoodArrays[currentPIndex];
				for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
				{
					const Storm::Vector3 gradKernel_NablaWij = gradKernel(k_kernelLength, neighbor._positionDifferenceVector, neighbor._vectToParticleNorm);

					const Storm::Vector3 vij = vi - neighbor._containingParticleSystem->getVelocity()[neighbor._particleIndex];

					const float vijDotXij = vij.dot(neighbor._positionDifferenceVector);
					const float viscoGlobalCoeff = currentPMass * 10.f * vijDotXij / (neighbor._vectToParticleSquaredNorm + viscoPrecoeff);

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

							// Mirror the force on the boundary solid following the 3rd newton law
							if (!neighborPSystemAsBoundary->isStatic())
							{
								Storm::Vector3 &boundaryNeighborForce = neighbor._containingParticleSystem->getForces()[neighbor._particleIndex];
								Storm::Vector3 &boundaryNeighborTmpViscosityForce = neighbor._containingParticleSystem->getTemporaryViscosityForces()[neighbor._particleIndex];

								std::lock_guard<std::mutex> lock{ neighbor._containingParticleSystem->_mutex };
								boundaryNeighborForce -= viscosityComponent;
								boundaryNeighborTmpViscosityForce -= viscosityComponent;
							}
						}
						else
						{
							viscosityComponent = Storm::Vector3::Zero();
						}
					}

					totalViscosityForceOnParticle += viscosityComponent;
				}

				currentPForce += totalViscosityForceOnParticle;

				temporaryPViscoForce[currentPIndex] = totalViscosityForceOnParticle;

				// We should also initialize the acceleration field now (avoid to restart the threads).
				dataField[currentPIndex]._nonPressureAcceleration = currentPForce / currentPMass;
			});
		}
	}

	const float totalParticleCountFl = static_cast<float>(_totalParticleCount);

	// 4th : The density prediction (pressure solving)
	do
	{
		// Initialize the components
		for (auto &particleSystemPair : particleSystems)
		{
			Storm::ParticleSystem &particleSystem = *particleSystemPair.second;
			if (!particleSystem.isFluids() && !particleSystem.isStatic())
			{
				Storm::runParallel(particleSystem.getTemporaryPressureForces(), [](Storm::Vector3 &pressuresForces) 
				{
					pressuresForces.setZero();
				});
			}
		}

		// Compute the acceleration field
		for (auto &dataFieldPair : _data)
		{
			// Since data field was made from fluids particles only, no need to check if this is a fluid.
			const Storm::FluidParticleSystem &currentParticleSystem = static_cast<const Storm::FluidParticleSystem &>(*particleSystems.find(dataFieldPair.first)->second);

			const std::vector<float> &masses = currentParticleSystem.getMasses();
			const std::vector<float> &densities = currentParticleSystem.getDensities();
			const std::vector<float> &pressures = currentParticleSystem.getPressures();
			const std::vector<Storm::ParticleNeighborhoodArray> &neighborhoodArrays = currentParticleSystem.getNeighborhoodArrays();

			Storm::runParallel(dataFieldPair.second, [&](Storm::PCISPHSolverData &currentPData, const std::size_t currentPIndex)
			{
				const Storm::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoodArrays[currentPIndex];

				const float currentPMass = masses[currentPIndex];
				const float currentPPressure = pressures[currentPIndex];
				const float currentPDensity = densities[currentPIndex];
				const float currentPPressureCoeff = currentPPressure / (currentPDensity * currentPDensity);

				currentPData._pressureAcceleration.setZero();

				for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
				{
					Storm::Vector3 pressureAccel;
					if (neighbor._containingParticleSystem->isFluids())
					{
						const Storm::FluidParticleSystem &neighborPSystemAsFluid = static_cast<const Storm::FluidParticleSystem &>(*neighbor._containingParticleSystem);
						
						const float neighborDensity = neighborPSystemAsFluid.getDensities()[neighbor._particleIndex];
						const float neighborPPressureCoeff = neighborPSystemAsFluid.getPressures()[neighbor._particleIndex] / (neighborDensity * neighborDensity);
						const float factor = neighborPSystemAsFluid.getMasses()[neighbor._particleIndex] * (currentPPressureCoeff + neighborPPressureCoeff);

						pressureAccel = factor * gradKernel(k_kernelLength, neighbor._positionDifferenceVector, neighbor._vectToParticleNorm);
					}
					else
					{
						Storm::RigidBodyParticleSystem &neighborPSystemAsRb = static_cast<Storm::RigidBodyParticleSystem &>(*neighbor._containingParticleSystem);
						const float factor = neighborPSystemAsRb.getVolumes()[neighbor._particleIndex] * currentPPressureCoeff;

						pressureAccel = factor * gradKernel(k_kernelLength, neighbor._positionDifferenceVector, neighbor._vectToParticleNorm);

						const Storm::Vector3 addedPressureForce = pressureAccel * currentPMass;
						Storm::Vector3 &pressureForce = neighborPSystemAsRb.getTemporaryPressureForces()[neighbor._particleIndex];

						std::lock_guard<std::mutex> lock{ neighborPSystemAsRb._mutex };
						pressureForce += addedPressureForce;
					}

					currentPData._pressureAcceleration -= pressureAccel;
				}
			});
		}

		// Compute the density and pressure changes
		for (auto &dataFieldPair : _data)
		{
			// Since data field was made from fluids particles only, no need to check if this is a fluid.
			Storm::FluidParticleSystem &currentParticleSystem = static_cast<Storm::FluidParticleSystem &>(*particleSystems.find(dataFieldPair.first)->second);

			std::vector<float> &predictedDensities = currentParticleSystem.getDensities();
			std::vector<float> &predictedPressures = currentParticleSystem.getPressures();
			const std::vector<Storm::ParticleNeighborhoodArray> &neighborhoodArrays = currentParticleSystem.getNeighborhoodArrays();

			const float k_restDensity = currentParticleSystem.getRestDensity();

			std::atomic<float> densityError = 0.f;

			Storm::runParallel(dataFieldPair.second, [&](Storm::PCISPHSolverData &currentPData, const std::size_t currentPIndex)
			{
				const Storm::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoodArrays[currentPIndex];

				float &currentPPredictedDensity = predictedDensities[currentPIndex];
				float &currentPPredictedPressure = predictedPressures[currentPIndex];

				const Storm::Vector3 currentPAccel = currentPData._nonPressureAcceleration + currentPData._pressureAcceleration;

				float currentPPressureAccelChangeCoeff = 0.f;
				for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
				{
					const Storm::Vector3 gradWij = gradKernel(k_kernelLength, neighbor._positionDifferenceVector, neighbor._vectToParticleNorm);
					if (neighbor._containingParticleSystem->isFluids())
					{
						const Storm::FluidParticleSystem &neighborPSystemAsFluid = static_cast<const Storm::FluidParticleSystem &>(*neighbor._containingParticleSystem);
						const Storm::PCISPHSolverData &associatedNeighborData = _data[neighborPSystemAsFluid.getId()][neighbor._particleIndex];
						const float accelGrad = (currentPAccel - (associatedNeighborData._nonPressureAcceleration + associatedNeighborData._pressureAcceleration)).dot(gradWij);

						currentPPressureAccelChangeCoeff += neighborPSystemAsFluid.getMasses()[neighbor._particleIndex] * accelGrad;
					}
					else
					{
						const Storm::RigidBodyParticleSystem &neighborPSystemAsRb = static_cast<const Storm::RigidBodyParticleSystem &>(*neighbor._containingParticleSystem);
						const float accelGrad = currentPAccel.dot(gradWij);
						currentPPressureAccelChangeCoeff += neighborPSystemAsRb.getVolumes()[neighbor._particleIndex] * k_restDensity * accelGrad;
					}
				}

				currentPPressureAccelChangeCoeff *= deltaTimeSquared;

				currentPPredictedPressure += k_templatePStiffnessCoeffK * (k_restDensity - currentPPredictedDensity - currentPPressureAccelChangeCoeff);
				currentPPredictedDensity = currentPPressureAccelChangeCoeff;

				densityError += k_restDensity * std::max(currentPPredictedDensity - 1.f, 0.f);
			});

			averageDensityError += densityError;
		}

		averageDensityError /= totalParticleCountFl;
	} while (averageDensityError > generalSimulData._maxDensityError && currentPredictionIter++ < generalSimulData._maxPredictIteration);

	if (currentPredictionIter > generalSimulData._maxPredictIteration)
	{
		LOG_DEBUG_WARNING <<
			"Max prediction loop watchdog hit without being able to go under the max density error allowed when computing PCISPH predicted density...\n"
			"We'll leave the prediction loop with an average density of " << averageDensityError;
	}

	for (auto &particleSystemPair : particleSystems)
	{
		Storm::ParticleSystem &particleSystem = *particleSystemPair.second;
		if (!particleSystem.isFluids() && !particleSystem.isStatic())
		{
			Storm::runParallel(particleSystem.getForces(), [&particleSystem](Storm::Vector3 &forces, const std::size_t currentPIndex)
			{
				forces = particleSystem.getTemporaryViscosityForces()[currentPIndex] + particleSystem.getTemporaryPressureForces()[currentPIndex];
			});
		}
	}

	for (auto &dataFieldPair : _data)
	{
		// Since data field was made from fluids particles only, no need to check if this is a fluid.
		Storm::FluidParticleSystem &currentParticleSystem = static_cast<Storm::FluidParticleSystem &>(*particleSystems.find(dataFieldPair.first)->second);

		const std::vector<float> &masses = currentParticleSystem.getMasses();
		std::vector<Storm::Vector3> &forces = currentParticleSystem.getForces();
		std::vector<Storm::Vector3> &tmpPressureForces = currentParticleSystem.getTemporaryPressureForces();

		Storm::runParallel(dataFieldPair.second, [&](Storm::PCISPHSolverData &currentPData, const std::size_t currentPIndex)
		{
			forces[currentPIndex] = (currentPData._nonPressureAcceleration + currentPData._pressureAcceleration) * masses[currentPIndex];
			tmpPressureForces[currentPIndex] = currentPData._pressureAcceleration * masses[currentPIndex];
		});
	}
}
