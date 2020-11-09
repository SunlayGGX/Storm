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


// Those are implementation settings...
// Because the implementation I used as a reference used a lot of hack that aren't described inside their tutorial/papers (maybe those are hacks, maybe those are mistakes, I don't know),
// I need to find an implementation that doesn't rely on those hacks.
#define STORM_USE_SPLISH_SPLASH_BIDOUILLE false
#define STORM_USE_SPLISH_SPLASH_DENSITY_BIDOUILLE true
#define STORM_MINUS_ACCEL false


Storm::PCISPHSolver::PCISPHSolver(const float k_kernelLength, const Storm::ParticleSystemContainer &particleSystemsMap) :
	_totalParticleCount{ 0 }
{
	// Compute the uniform stiffness k_PCI coefficient
	Storm::Vector3d uniformNeighborStiffnessCoeffSum = Storm::Vector3d::Zero();
	double uniformNeighborStiffnessCoeffProd = 0.0;

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
	const Storm::GeneralSimulationData &generalSimulData = configMgr.getGeneralSimulationData();

	const float k_kernelLengthSquared = k_kernelLength * k_kernelLength;

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

				const float normSquared = neighborUniformPos.squaredNorm();
				if (Storm::ParticleSystem::isElligibleNeighborParticle(k_kernelLengthSquared, normSquared))
				{
					const Storm::Vector3 gradVal = k_gradMethod(k_kernelLength, neighborUniformPos, std::sqrtf(normSquared));

					// The values goes quickly really high, therefore k_templatePVolumeSquared is to reduce them to prevent going into the inaccurate values of the data type.
					uniformNeighborStiffnessCoeffSum += gradVal.cast<double>() * k_templatePVolumeSquared;
					uniformNeighborStiffnessCoeffProd += gradVal.squaredNorm() * k_templatePVolumeSquared;
				}
			}
		}
	}

	_kUniformStiffnessConstCoefficient = static_cast<float>(-0.5 / (uniformNeighborStiffnessCoeffSum.squaredNorm() + uniformNeighborStiffnessCoeffProd));

	// Initialize the pcisph data field
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
	const float k_kernelLengthSquared = k_kernelLength * k_kernelLength;

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
			const std::vector<Storm::ParticleNeighborhoodArray> &neighborhoodArrays = fluidParticleSystem.getNeighborhoodArrays();
			std::vector<float> &pressures = fluidParticleSystem.getPressures();

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
					currentPPressure = fluidConfigData._kPressureStiffnessCoeff * (std::powf(currentPDensity / density0, fluidConfigData._kPressureExponentCoeff) - 1.f);
				}
			});
		}
	}

	// 3rd : Compute the non pressure forces (viscosity)
	this->computeNonPressureForce(particleSystems, k_kernelLength, generalSimulData, fluidConfigData, [this](unsigned int pSystemIndex)
	{
		return &_data.find(pSystemIndex)->second;
	}, [](void* datafield, const std::size_t currentPIndex, const Storm::FluidParticleSystem &currentParticleSystem)
	{
		Storm::PCISPHSolverData &currentPDataField = (*reinterpret_cast<std::vector<Storm::PCISPHSolverData>*>(datafield))[currentPIndex];

		currentPDataField._nonPressureAcceleration = currentParticleSystem.getForces()[currentPIndex] / currentParticleSystem.getMasses()[currentPIndex];
		currentPDataField._predictedAcceleration = currentPDataField._nonPressureAcceleration;
		currentPDataField._srcPosition = currentParticleSystem.getPositions()[currentPIndex];
		currentPDataField._srcVelocity = currentParticleSystem.getVelocity()[currentPIndex];
		currentPDataField._predictedPressure = 0.f;
	});

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
				// For all dynamic rigid bodies, reset the temporary pressure force since we don't have temporary data for them.
				Storm::runParallel(particleSystem.getTemporaryPressureForces(), [](Storm::Vector3 &pressuresForces) 
				{
					pressuresForces.setZero();
				});
			}
		}

		averageDensityError = 0.f;

		// Project the positions and velocities
		for (auto &dataFieldPair : _data)
		{
			Storm::runParallel(dataFieldPair.second, [&](Storm::PCISPHSolverData &currentPData, const std::size_t currentPIndex)
			{
				currentPData._currentVelocity = currentPData._srcVelocity + currentPData._predictedAcceleration * k_deltaTime;
				currentPData._currentPosition = currentPData._srcPosition + currentPData._currentVelocity * k_deltaTime;
			});
		}

		// Compute predicted density and predicted pressure
		for (auto &dataFieldPair : _data)
		{
			// Since data field was made from fluids particles only, no need to check if this is a fluid.
			const Storm::FluidParticleSystem &fluidParticleSystem = static_cast<const Storm::FluidParticleSystem &>(*particleSystems.find(dataFieldPair.first)->second);

			const std::vector<float> &masses = fluidParticleSystem.getMasses();
			const std::vector<float> &densities = fluidParticleSystem.getDensities();
			const std::vector<Storm::ParticleNeighborhoodArray> &neighborhoodArrays = fluidParticleSystem.getNeighborhoodArrays();

			const float density0 = fluidParticleSystem.getRestDensity();

			std::atomic<float> densityError = 0.f;

			Storm::runParallel(dataFieldPair.second, [&](Storm::PCISPHSolverData &currentPData, const std::size_t currentPIndex)
			{
#if !STORM_USE_SPLISH_SPLASH_BIDOUILLE
				const float currentPMass = masses[currentPIndex];
				const float currentPDensity = densities[currentPIndex];
				const float currentPVolume = currentPMass / currentPDensity;

				currentPData._predictedDensity = currentPVolume * k_kernelZero;
#else // SplishSplash impl
				currentPData._predictedDensity = fluidParticleSystem.getParticleVolume() * k_kernelZero;
#endif

				const std::pair<const unsigned int, std::vector<Storm::PCISPHSolverData>>* currentNeighborPFluidData = &*std::begin(_data);

				const Storm::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoodArrays[currentPIndex];
				for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
				{
					float neighborPVolume;
					float rawW;

					if (neighbor._isFluidParticle)
					{
						const Storm::FluidParticleSystem &neighborFluidParticleSystem = static_cast<const Storm::FluidParticleSystem &>(*neighbor._containingParticleSystem);

						const unsigned int neighborParticleSystemIndex = neighborFluidParticleSystem.getId();
						if (currentNeighborPFluidData->first != neighborParticleSystemIndex)
						{
							currentNeighborPFluidData = &*_data.find(neighborParticleSystemIndex);
						}

						const Storm::Vector3 xij = currentPData._currentPosition - currentNeighborPFluidData->second[neighbor._particleIndex]._currentPosition;
						
						const float normSquared = xij.squaredNorm();
						if (Storm::ParticleSystem::isElligibleNeighborParticle(k_kernelLengthSquared, normSquared))
						{
#if !STORM_USE_SPLISH_SPLASH_BIDOUILLE
							const float neighborPMass = neighborFluidParticleSystem.getMasses()[neighbor._particleIndex];
							const float neighborPDensity = neighborFluidParticleSystem.getDensities()[neighbor._particleIndex];

							neighborPVolume = neighborPMass / neighborPDensity;
#else // SplishSplash impl
							neighborPVolume = neighborFluidParticleSystem.getParticleVolume();
#endif

							rawW = rawKernel(k_kernelLength, std::sqrtf(normSquared));
							currentPData._predictedDensity += neighborPVolume * rawW;
						}
					}
					else
					{
						const Storm::RigidBodyParticleSystem &neighborRbParticleSystem = static_cast<const Storm::RigidBodyParticleSystem &>(*neighbor._containingParticleSystem);

						const Storm::Vector3 xij = currentPData._currentPosition - neighborRbParticleSystem.getPositions()[neighbor._particleIndex];
						const float normSquared = xij.squaredNorm();
						if (Storm::ParticleSystem::isElligibleNeighborParticle(k_kernelLengthSquared, normSquared))
						{
							neighborPVolume = neighborRbParticleSystem.getVolumes()[neighbor._particleIndex];

							rawW = rawKernel(k_kernelLength, std::sqrtf(normSquared));
							currentPData._predictedDensity += neighborPVolume * rawW;
						}
					}
				}

#if !STORM_USE_SPLISH_SPLASH_DENSITY_BIDOUILLE
				densityError += std::fabs((currentPData._predictedDensity - density0) / density0);
				currentPData._predictedPressure += k_templatePStiffnessCoeffK * currentPData._predictedDensity;
#else // SplishSplash impl

				currentPData._predictedDensity = std::max(currentPData._predictedDensity, 1.f);
				const float shiftedPredictedDensity = currentPData._predictedDensity - 1.f;
				densityError += density0 * shiftedPredictedDensity;
				currentPData._predictedPressure += k_templatePStiffnessCoeffK * shiftedPredictedDensity;
#endif
			});

			averageDensityError += densityError;
		}

		averageDensityError /= totalParticleCountFl;

		// Compute the acceleration field
		for (auto &dataFieldPair : _data)
		{
			// Since data field was made from fluids particles only, no need to check if this is a fluid.
			const Storm::FluidParticleSystem &currentParticleSystem = static_cast<const Storm::FluidParticleSystem &>(*particleSystems.find(dataFieldPair.first)->second);

			const std::vector<float> &masses = currentParticleSystem.getMasses();
			const std::vector<Storm::ParticleNeighborhoodArray> &neighborhoodArrays = currentParticleSystem.getNeighborhoodArrays();

			Storm::runParallel(dataFieldPair.second, [&](Storm::PCISPHSolverData &currentPData, const std::size_t currentPIndex)
			{
				const Storm::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoodArrays[currentPIndex];
				currentPData._predictedAcceleration = currentPData._nonPressureAcceleration;

				const std::pair<const unsigned int, std::vector<Storm::PCISPHSolverData>>* currentNeighborPFluidData = &*std::begin(_data);

#if !STORM_USE_SPLISH_SPLASH_BIDOUILLE
				const Storm::Vector3 &xi = currentPData._currentPosition;
#else // SplishSplash impl
				const Storm::Vector3 &xi = currentPData._srcPosition;
#endif

				for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
				{
					if (neighbor._isFluidParticle)
					{
						const Storm::FluidParticleSystem &neighborPSystemAsFluid = static_cast<const Storm::FluidParticleSystem &>(*neighbor._containingParticleSystem);

						const unsigned int neighborParticleSystemIndex = neighborPSystemAsFluid.getId();
						if (currentNeighborPFluidData->first != neighborParticleSystemIndex)
						{
							currentNeighborPFluidData = &*_data.find(neighborParticleSystemIndex);
						}

						const Storm::PCISPHSolverData &neighborPData = currentNeighborPFluidData->second[neighbor._particleIndex];

						float neighborPVolume;

#if !STORM_USE_SPLISH_SPLASH_BIDOUILLE
						const float neighborPMass = neighborPSystemAsFluid.getMasses()[neighbor._particleIndex];
						const float neighborPDensity = neighborPSystemAsFluid.getDensities()[neighbor._particleIndex];

						neighborPVolume = neighborPMass / neighborPDensity;
#else // SplishSplash impl
						neighborPVolume = neighborPSystemAsFluid.getParticleVolume();
#endif

#if !STORM_USE_SPLISH_SPLASH_BIDOUILLE
						const float pressureCoeff = neighborPVolume * ((currentPData._predictedPressure / (currentPData._predictedDensity * currentPData._predictedDensity)) + neighborPData._predictedPressure);
#else // SplishSplash impl
						const float pressureCoeff = neighborPVolume * (currentPData._predictedPressure + neighborPData._predictedPressure);
#endif

						const Storm::Vector3 &xij = xi - neighborPData._currentPosition;

						const float normSquared = xij.squaredNorm();
						if (Storm::ParticleSystem::isElligibleNeighborParticle(k_kernelLengthSquared, normSquared))
						{
#if STORM_MINUS_ACCEL
							currentPData._predictedAcceleration -= pressureCoeff * gradKernel(k_kernelLength, xij, std::sqrtf(normSquared));
#else
							currentPData._predictedAcceleration += pressureCoeff * gradKernel(k_kernelLength, xij, std::sqrtf(normSquared));
#endif
						}
					}
					else
					{
						Storm::RigidBodyParticleSystem &neighborPSystemAsRb = static_cast<Storm::RigidBodyParticleSystem &>(*neighbor._containingParticleSystem);
						const float neighborPVolume = neighborPSystemAsRb.getVolumes()[neighbor._particleIndex];

						const float pressureCoeff = neighborPVolume * currentPData._predictedPressure;
						const Storm::Vector3 xij = xi - neighborPSystemAsRb.getPositions()[neighbor._particleIndex];

						const float normSquared = xij.squaredNorm();
						if (Storm::ParticleSystem::isElligibleNeighborParticle(k_kernelLengthSquared, normSquared))
						{
							// An acceleration
							Storm::Vector3 pressureTmpVect = pressureCoeff * gradKernel(k_kernelLength, xij, std::sqrtf(normSquared));

#if STORM_MINUS_ACCEL
							currentPData._predictedAcceleration -= pressureTmpVect;
#else
							currentPData._predictedAcceleration += pressureTmpVect;
#endif

							if (!neighborPSystemAsRb.isStatic())
							{
								// Now a force
								pressureTmpVect *= (neighborPSystemAsRb.getVolumes()[neighbor._particleIndex] * currentPData._predictedDensity);

								Storm::Vector3 &tmpPressureForce = neighborPSystemAsRb.getTemporaryPressureForces()[neighbor._particleIndex];

								std::lock_guard<std::mutex> lock{ neighbor._containingParticleSystem->_mutex };
#if STORM_MINUS_ACCEL
								tmpPressureForce += pressureTmpVect;
#else
								tmpPressureForce -= pressureTmpVect;
#endif
							}
						}
					}
				}
			});
		}

	} while (currentPredictionIter++ < generalSimulData._maxPredictIteration && averageDensityError > generalSimulData._maxDensityError);

	this->updateCurrentPredictionIter(currentPredictionIter, generalSimulData._maxPredictIteration, averageDensityError, generalSimulData._maxDensityError);

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
		std::vector<float> &densities = currentParticleSystem.getDensities();
		std::vector<float> &pressures = currentParticleSystem.getPressures();
		std::vector<Storm::Vector3> &forces = currentParticleSystem.getForces();
		std::vector<Storm::Vector3> &tmpPressureForces = currentParticleSystem.getTemporaryPressureForces();

		Storm::runParallel(dataFieldPair.second, [&](const Storm::PCISPHSolverData &currentPData, const std::size_t currentPIndex)
		{
			densities[currentPIndex] = currentPData._predictedDensity;
			pressures[currentPIndex] = currentPData._predictedPressure;

			const float currentPMass = masses[currentPIndex];
			forces[currentPIndex] = currentPData._predictedAcceleration * currentPMass;
			tmpPressureForces[currentPIndex] = (currentPData._predictedAcceleration - currentPData._nonPressureAcceleration) * currentPMass;
		});
	}
}
