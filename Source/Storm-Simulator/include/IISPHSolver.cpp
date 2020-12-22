#include "IISPHSolver.h"

#include "IISPHSolverData.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "SimulatorManager.h"

#include "FluidParticleSystem.h"
#include "RigidBodyParticleSystem.h"

#include "RunnerHelper.h"

#include "Kernel.h"

#include "GeneralSimulationData.h"
#include "FluidData.h"

#include "IterationParameter.h"

#define STORM_HIJACKED_TYPE Storm::IISPHSolverData
#	include "VectHijack.h"
#undef STORM_HIJACKED_TYPE


namespace
{
#define STORM_SOLVER_NAMES_XMACRO \
	STORM_SOLVER_NAME("Pressure")

	using GUINames = std::remove_reference_t<Storm::PredictiveSolverHandler::SolversNames>;

	constexpr static GUINames g_solverIterationNames
	{
#define STORM_SOLVER_NAME(SolverName) STORM_TEXT(SolverName " solve iteration"),
		STORM_SOLVER_NAMES_XMACRO
#undef STORM_SOLVER_NAME

		nullptr
	};

	constexpr static GUINames g_solverErrorsNames
	{
#define STORM_SOLVER_NAME(SolverName) STORM_TEXT(SolverName " solve error"),
		STORM_SOLVER_NAMES_XMACRO
#undef STORM_SOLVER_NAME

		nullptr
	};

#undef STORM_SOLVER_NAMES_XMACRO
}


Storm::IISPHSolver::IISPHSolver(const float k_kernelLength, const Storm::ParticleSystemContainer &particleSystemsMap) :
	Storm::PredictiveSolverHandler{ g_solverIterationNames, g_solverErrorsNames }
{
	std::size_t totalParticleCount = 0;

	for (const auto &particleSystemPair : particleSystemsMap)
	{
		const Storm::ParticleSystem &currentPSystem = *particleSystemPair.second;
		if (currentPSystem.isFluids())
		{
			const Storm::VectorHijacker currentPSystemPCount{ currentPSystem.getParticleCount() };

			std::vector<Storm::IISPHSolverData> &currentPSystemData = _data[particleSystemPair.first];
			currentPSystemData.reserve(currentPSystemPCount._newSize);
			Storm::setNumUninitialized_hijack(currentPSystemData, currentPSystemPCount);

			totalParticleCount += currentPSystemPCount._newSize;
		}
	}

	_totalParticleCountFl = static_cast<float>(totalParticleCount);
}

void Storm::IISPHSolver::execute(const Storm::IterationParameter &iterationParameter)
{
	// Note :
	// Even if some part of the algorithm is exactly the same as inside other solvers, I did not factorize on purpose (I did, but reverted immediately because it was a really bad idea) !
	// The reason is that the algorithm piece works for this solver. If a bug arise, then it could be because of this solver algorithm and don't have anything to do with other solvers algorithm,
	// therefore trying to fix the parent factorized method is not the right solution since it would risk to jeopardize all other solvers.
	//
	// Yes I know it is hard to maintain with all those copy-pasted piece of code, but it would be harder to improve/develop a specific solver where all modifications are shared and could break other solvers we didn't test (since I don't have any QA and don't have time to test every solvers myself, it is preferable to keep copy pasted code).
	// Therefore, if you detect a bug in any solvers, and think the bug would impact other solvers, check them manually one by one and fix the issue locally.

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
	const Storm::GeneralSimulationData &generalSimulData = configMgr.getGeneralSimulationData();
	const Storm::FluidData &fluidConfigData = configMgr.getFluidData();
	
	Storm::SimulatorManager &simulMgr = Storm::SimulatorManager::instance();

	const float k_kernelZero = Storm::retrieveKernelZeroValue(generalSimulData._kernelMode);
	const Storm::RawKernelMethodDelegate rawKernel = Storm::retrieveRawKernelMethod(generalSimulData._kernelMode);
	const Storm::GradKernelMethodDelegate gradKernel = Storm::retrieveGradKernelMethod(generalSimulData._kernelMode);

	const float k_maxDensityError = generalSimulData._maxDensityError;
	unsigned int currentPredictionIter = 0;

	const float deltaTimeSquared = iterationParameter._deltaTime * iterationParameter._deltaTime;
	const float k_kernelLengthSquared = iterationParameter._kernelLength * iterationParameter._kernelLength;

	Storm::ParticleSystemContainer &particleSystems = *iterationParameter._particleSystems;

	// 1st : Initialize iteration
	simulMgr.advanceBlowersTime(iterationParameter._deltaTime);
	simulMgr.refreshParticleNeighborhood();
	simulMgr.subIterationStart();

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

			Storm::runParallel(fluidParticleSystem.getDensities(), [&](float &currentPDensity, const std::size_t currentPIndex)
			{
				// Density
				currentPDensity = particleVolume * k_kernelZero;

				const Storm::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoodArrays[currentPIndex];
				for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
				{
					const float kernelValue_Wij = rawKernel(iterationParameter._kernelLength, neighbor._vectToParticleNorm);
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
			const float particleVolume = fluidParticleSystem.getParticleVolume();
			const std::vector<Storm::ParticleNeighborhoodArray> &neighborhoodArrays = currentParticleSystem.getNeighborhoodArrays();
			std::vector<float> &masses = fluidParticleSystem.getMasses();
			const std::vector<float> &densities = fluidParticleSystem.getDensities();
			std::vector<Storm::Vector3> &temporaryPViscoForces = fluidParticleSystem.getTemporaryViscosityForces();

			const std::vector<Storm::Vector3> &velocities = fluidParticleSystem.getVelocity();

			const float viscoPrecoeff = 0.01f * k_kernelLengthSquared;

			std::vector<Storm::IISPHSolverData> &dataField = _data.find(particleSystemPair.first)->second;

			Storm::runParallel(fluidParticleSystem.getForces(), [&](Storm::Vector3 &currentPForce, const std::size_t currentPIndex)
			{
				float &currentPMass = masses[currentPIndex];
				const float currentPDensity = densities[currentPIndex];
				const Storm::Vector3 &vi = velocities[currentPIndex];

				currentPMass = particleVolume * currentPDensity;

				const float restMassDensity = currentPMass * density0;

				Storm::Vector3 totalViscosityForceOnParticle = Storm::Vector3::Zero();

				const Storm::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoodArrays[currentPIndex];
				for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
				{
					const Storm::Vector3 gradKernel_NablaWij = gradKernel(iterationParameter._kernelLength, neighbor._positionDifferenceVector, neighbor._vectToParticleNorm);

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
						}
						else
						{
							viscosityComponent = Storm::Vector3::Zero();
						}

						// Mirror the force on the boundary solid following the 3rd newton law
						if (!neighborPSystemAsBoundary->isStatic())
						{
							Storm::Vector3 &boundaryNeighborTmpViscosityForce = neighbor._containingParticleSystem->getTemporaryViscosityForces()[neighbor._particleIndex];

							std::lock_guard<std::mutex> lock{ neighbor._containingParticleSystem->_mutex };
							boundaryNeighborTmpViscosityForce -= viscosityComponent;
						}
					}

					totalViscosityForceOnParticle += viscosityComponent;
				}

				currentPForce += totalViscosityForceOnParticle;

				temporaryPViscoForces[currentPIndex] = totalViscosityForceOnParticle;

				// We should also initialize the data field now (avoid to restart the threads).
				Storm::IISPHSolverData &currentPDataField = dataField[currentPIndex];

				currentPDataField._nonPressureAcceleration = currentPForce / currentPMass;
				currentPDataField._predictedAcceleration = currentPDataField._nonPressureAcceleration;
				currentPDataField._predictedVelocity = vi + currentPDataField._nonPressureAcceleration * iterationParameter._deltaTime;

				// Note : Maybe we should also compute a prediction of the position ?
			});
		}
	}

	// 4th : compute advection coefficients
	for (auto &dataFieldPair : _data)
	{
		// Since data field was made from fluids particles only, no need to check if this is a fluid.
		Storm::FluidParticleSystem &fluidParticleSystem = static_cast<Storm::FluidParticleSystem &>(*particleSystems.find(dataFieldPair.first)->second);

		const std::vector<float> &masses = fluidParticleSystem.getMasses();
		const std::vector<float> &densities = fluidParticleSystem.getDensities();
		const std::vector<Storm::ParticleNeighborhoodArray> &neighborhoodArrays = fluidParticleSystem.getNeighborhoodArrays();
		std::vector<float> &pressures = fluidParticleSystem.getPressures();

		const float density0 = fluidParticleSystem.getRestDensity();

		const float fluidDefaultPVolume = fluidParticleSystem.getParticleVolume();

		Storm::runParallel(dataFieldPair.second, [&](Storm::IISPHSolverData &currentPData, const std::size_t currentPIndex)
		{
			const Storm::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoodArrays[currentPIndex];

			// Compute d_ii
			const float currentPDensityRatio = densities[currentPIndex] / density0;
			const float currentPDensityRatioSquared = currentPDensityRatio * currentPDensityRatio;

			currentPData._dii.setZero();

			// To optimize, compute fluid neighbors dii coefficient separately.
			Storm::Vector3 fluidDiiTmp = Storm::Vector3::Zero();
			for (const Storm::NeighborParticleInfo &neighborInfo : currentPNeighborhood)
			{
				const Storm::Vector3 gradientWij = gradKernel(iterationParameter._kernelLength, neighborInfo._positionDifferenceVector, neighborInfo._vectToParticleNorm);
				if (neighborInfo._isFluidParticle)
				{
					fluidDiiTmp -= gradientWij;
				}
				else
				{
					const Storm::RigidBodyParticleSystem &neighborRbSystem = static_cast<const Storm::RigidBodyParticleSystem &>(*neighborInfo._containingParticleSystem);
					const float neighborPVolume = neighborRbSystem.getVolumes()[neighborInfo._particleIndex];
					currentPData._dii -= (neighborPVolume / currentPDensityRatioSquared) * gradientWij;
				}
			}

			const float fluidDpiCoeff = fluidDefaultPVolume / currentPDensityRatioSquared;
			currentPData._dii += fluidDpiCoeff * fluidDiiTmp;

			// Compute advection density and aii
			currentPData._advectedDensity = 0.f;
			currentPData._aii = 0.f;

			Storm::Vector3 diffVelocity;

			const std::vector<Storm::IISPHSolverData>* neighborDataArray = &dataFieldPair.second;
			const Storm::FluidParticleSystem* lastNeighborFluidSystem = &fluidParticleSystem;

			float neighborPVolume;

			for (const Storm::NeighborParticleInfo &neighborInfo : currentPNeighborhood)
			{
				const Storm::Vector3 gradientWij = gradKernel(iterationParameter._kernelLength, neighborInfo._positionDifferenceVector, neighborInfo._vectToParticleNorm);

				if (neighborInfo._isFluidParticle)
				{
					if (neighborInfo._containingParticleSystem != lastNeighborFluidSystem)
					{
						lastNeighborFluidSystem = static_cast<const Storm::FluidParticleSystem*>(neighborInfo._containingParticleSystem);
						neighborDataArray = &_data.find(lastNeighborFluidSystem->getId())->second;
					}

					const Storm::IISPHSolverData &neighborData = (*neighborDataArray)[neighborInfo._particleIndex];

					neighborPVolume = lastNeighborFluidSystem->getParticleVolume();
					diffVelocity = currentPData._predictedVelocity - neighborData._predictedVelocity;
				}
				else
				{
					const Storm::RigidBodyParticleSystem &neighborRbSystem = static_cast<const Storm::RigidBodyParticleSystem &>(*neighborInfo._containingParticleSystem);

					neighborPVolume = neighborRbSystem.getVolumes()[neighborInfo._particleIndex];
					diffVelocity = currentPData._predictedVelocity - neighborRbSystem.getVelocity()[neighborInfo._particleIndex];
				}

				currentPData._advectedDensity += neighborPVolume * diffVelocity.dot(gradientWij);
				currentPData._aii += neighborPVolume * (currentPData._dii - fluidDpiCoeff * gradientWij).dot(gradientWij);
			}

			currentPData._advectedDensity *= iterationParameter._deltaTime;
			currentPData._advectedDensity += currentPDensityRatio;

			// Init pressures
			float &currentPPressure = pressures[currentPIndex];

			// From the original 2014 article (the one written by Ihmsen, not the tutorial). IISPH initialized the pressure to 0.5 of the pressure from last simulation step.
			// It was an empirical value motivated by greater convergence, the downside than the nearest it is from 0, the slower the convergence would get.
			currentPPressure *= fluidConfigData._pressureInitRelaxationCoefficient;

			currentPData._diiP = currentPData._dii * currentPPressure;
		});
	}

	// 5th : start prediction
	float averageDensityError;

	do 
	{
		// Initialize prediction iteration
		this->initializePredictionIteration(particleSystems, averageDensityError);

		// Compute dijPj
		for (auto &dataFieldPair : _data)
		{
			// Since data field was made from fluids particles only, no need to check if this is a fluid.
			const Storm::FluidParticleSystem &fluidParticleSystem = static_cast<const Storm::FluidParticleSystem &>(*particleSystems.find(dataFieldPair.first)->second);

			const std::vector<Storm::ParticleNeighborhoodArray> &neighborhoodArrays = fluidParticleSystem.getNeighborhoodArrays();

			Storm::runParallel(dataFieldPair.second, [&](Storm::IISPHSolverData &currentPData, const std::size_t currentPIndex)
			{
				const Storm::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoodArrays[currentPIndex];

				currentPData._dijPj.setZero();

				for (const Storm::NeighborParticleInfo &neighborInfo : currentPNeighborhood)
				{
					// Only fluid particles are used to compute dijPj
					if (neighborInfo._isFluidParticle)
					{
						const Storm::FluidParticleSystem &neighborPSystemAsFluid = static_cast<const Storm::FluidParticleSystem &>(*neighborInfo._containingParticleSystem);

						const float neighborDensityCoeffRatio = neighborPSystemAsFluid.getDensities()[neighborInfo._particleIndex] / neighborPSystemAsFluid.getRestDensity();
						const float dpj = neighborDensityCoeffRatio * neighborDensityCoeffRatio;

						const float coeff = neighborPSystemAsFluid.getParticleVolume() / dpj * neighborPSystemAsFluid.getPressures()[neighborInfo._particleIndex];
						const Storm::Vector3 gradientWij = gradKernel(iterationParameter._kernelLength, neighborInfo._positionDifferenceVector, neighborInfo._vectToParticleNorm);

						currentPData._dijPj -= coeff * gradientWij;
					}
				}
			});
		}

		// Compute new advected pressure
		for (auto &dataFieldPair : _data)
		{
			// Since data field was made from fluids particles only, no need to check if this is a fluid.
			const Storm::FluidParticleSystem &fluidParticleSystem = static_cast<const Storm::FluidParticleSystem &>(*particleSystems.find(dataFieldPair.first)->second);

			const std::vector<float> &densities = fluidParticleSystem.getDensities();
			const std::vector<float> &pressures = fluidParticleSystem.getPressures();
			const std::vector<Storm::ParticleNeighborhoodArray> &neighborhoodArrays = fluidParticleSystem.getNeighborhoodArrays();

			const float currentPSystemDensity0 = fluidParticleSystem.getRestDensity();

			std::atomic<float> densityErrorAccu = 0.f;

			Storm::runParallel(dataFieldPair.second, [&](Storm::IISPHSolverData &currentPData, const std::size_t currentPIndex)
			{
				const float &currentPPressure = pressures[currentPIndex];
				const float &currentPDensity = densities[currentPIndex];
				const Storm::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoodArrays[currentPIndex];

				const float currentPSystemDensityRatio = currentPDensity / currentPSystemDensity0;
				const float dpi = currentPPressure * fluidParticleSystem.getParticleVolume() / (currentPSystemDensityRatio * currentPSystemDensityRatio);

				const std::vector<Storm::IISPHSolverData>* neighborDataArray = &dataFieldPair.second;
				const Storm::FluidParticleSystem* lastNeighborFluidSystem = &fluidParticleSystem;
				float fluidNeighborPVolume = lastNeighborFluidSystem->getParticleVolume();

				float tmpSum = 0.f;

				for (const Storm::NeighborParticleInfo &neighborInfo : currentPNeighborhood)
				{
					// Note : Maybe use a predicted position instead.
					const Storm::Vector3 gradientWij = gradKernel(iterationParameter._kernelLength, neighborInfo._positionDifferenceVector, neighborInfo._vectToParticleNorm);

					if (neighborInfo._isFluidParticle)
					{
						if (neighborInfo._containingParticleSystem != lastNeighborFluidSystem)
						{
							lastNeighborFluidSystem = static_cast<const Storm::FluidParticleSystem*>(neighborInfo._containingParticleSystem);
							fluidNeighborPVolume = lastNeighborFluidSystem->getParticleVolume();
							neighborDataArray = &_data.find(lastNeighborFluidSystem->getId())->second;
						}

						const Storm::IISPHSolverData &neighborData = (*neighborDataArray)[neighborInfo._particleIndex];

						const Storm::Vector3 djiPi = dpi * gradientWij;
						const Storm::Vector3 sumVectDiv = currentPData._dijPj - neighborData._diiP - neighborData._dijPj + djiPi;

						tmpSum += fluidNeighborPVolume * sumVectDiv.dot(gradientWij);
					}
					else
					{
						const Storm::RigidBodyParticleSystem &neighborRbSystem = static_cast<const Storm::RigidBodyParticleSystem &>(*neighborInfo._containingParticleSystem);
						tmpSum += neighborRbSystem.getVolumes()[neighborInfo._particleIndex] * currentPData._dijPj.dot(gradientWij);
					}
				}

				const float denom = currentPData._aii * deltaTimeSquared;
				if (std::fabs(denom) > 0.000000001f)
				{
					const float densityAdvectionReverse = 1.f - currentPData._advectedDensity;

					currentPData._predictedPressure =
						fluidConfigData._relaxationCoefficient * (densityAdvectionReverse - deltaTimeSquared * tmpSum) / denom +
						currentPPressure * (1.f - fluidConfigData._relaxationCoefficient)
						;

					if (currentPData._predictedPressure > 0.f)
					{
						const float densityErrorAdded = currentPSystemDensity0 * ((currentPData._aii * currentPData._predictedPressure + tmpSum) * deltaTimeSquared - densityAdvectionReverse);
						densityErrorAccu += densityErrorAdded;
					}
					else
					{
						currentPData._predictedPressure = 0.f;
					}
				}
				else
				{
					currentPData._predictedPressure = 0.f;
				}
			});

			averageDensityError += densityErrorAccu;
		}

		averageDensityError /= _totalParticleCountFl;

		// Post iteration updates
		for (auto &dataFieldPair : _data)
		{
			// Since data field was made from fluids particles only, no need to check if this is a fluid.
			Storm::FluidParticleSystem &fluidParticleSystem = static_cast<Storm::FluidParticleSystem &>(*particleSystems.find(dataFieldPair.first)->second);
			std::vector<float> &pressures = fluidParticleSystem.getPressures();

			Storm::runParallel(dataFieldPair.second, [&pressures](Storm::IISPHSolverData &currentPData, const std::size_t currentPIndex)
			{
				float &currentPPressure = pressures[currentPIndex];
				currentPPressure = currentPData._predictedPressure;
				currentPData._diiP = currentPData._dii * currentPPressure;
			});
		}

		++currentPredictionIter;

	} while (currentPredictionIter < generalSimulData._minPredictIteration || (currentPredictionIter < generalSimulData._maxPredictIteration && averageDensityError > generalSimulData._maxDensityError));

	this->updateCurrentPredictionIter(currentPredictionIter, generalSimulData._maxPredictIteration, averageDensityError, generalSimulData._maxDensityError, 0);

	// 6th : Compute the pressure force
	for (auto &dataFieldPair : _data)
	{
		// Since data field was made from fluids particles only, no need to check if this is a fluid.
		const Storm::FluidParticleSystem &fluidParticleSystem = static_cast<const Storm::FluidParticleSystem &>(*particleSystems.find(dataFieldPair.first)->second);

		const std::vector<float> &densities = fluidParticleSystem.getDensities();
		const std::vector<float> &pressures = fluidParticleSystem.getPressures();
		const float currentPSystemDensity0 = fluidParticleSystem.getRestDensity();
		const std::vector<Storm::ParticleNeighborhoodArray> &neighborhoodArrays = fluidParticleSystem.getNeighborhoodArrays();

		Storm::runParallel(dataFieldPair.second, [&](Storm::IISPHSolverData &currentPData, const std::size_t currentPIndex)
		{
			const Storm::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoodArrays[currentPIndex];
			const float currentPDensity = densities[currentPIndex];

			currentPData._nonPressureAcceleration.setZero();

			const float currentPDensityRatio = currentPDensity / currentPSystemDensity0;
			const float dpi = pressures[currentPIndex] / (currentPDensityRatio * currentPDensityRatio);

			Storm::Vector3 tmpAccel;
			for (const Storm::NeighborParticleInfo &neighborInfo : currentPNeighborhood)
			{
				tmpAccel = gradKernel(iterationParameter._kernelLength, neighborInfo._positionDifferenceVector, neighborInfo._vectToParticleNorm);
				if (neighborInfo._isFluidParticle)
				{
					const Storm::FluidParticleSystem &neighborPSystemAsFluid = static_cast<const Storm::FluidParticleSystem &>(*neighborInfo._containingParticleSystem);

					const float neighborDensity0 = neighborPSystemAsFluid.getRestDensity();

					const float neighborDensityRatio = neighborPSystemAsFluid.getDensities()[neighborInfo._particleIndex] / neighborDensity0;
					const float restDensityRatio = neighborDensity0 / currentPSystemDensity0;

					const float dpj = restDensityRatio * neighborPSystemAsFluid.getPressures()[neighborInfo._particleIndex] / (neighborDensityRatio * neighborDensityRatio);

					tmpAccel *= neighborPSystemAsFluid.getParticleVolume() * (dpi + dpj);
				}
				else
				{
					Storm::RigidBodyParticleSystem &neighborPSystemAsRb = static_cast<Storm::RigidBodyParticleSystem &>(*neighborInfo._containingParticleSystem);
					tmpAccel *= neighborPSystemAsRb.getVolumes()[neighborInfo._particleIndex] * dpi;

					if (!neighborPSystemAsRb.isStatic())
					{
						const float neighborPRbMass = neighborPSystemAsRb.getVolumes()[neighborInfo._particleIndex] * currentPDensity;
						const Storm::Vector3 pressureForceOnRb = neighborPRbMass * tmpAccel;

						Storm::Vector3 &tmpPressureForce = neighborPSystemAsRb.getTemporaryPressureForces()[neighborInfo._particleIndex];

						std::lock_guard<std::mutex> lock{ neighborPSystemAsRb._mutex };
						tmpPressureForce += pressureForceOnRb;
					}
				}

				currentPData._predictedAcceleration -= tmpAccel;
			}
		});
	}

	this->transfertEndDataToSystems(particleSystems, iterationParameter, &_data, [](void* data, const unsigned int pSystemId, Storm::FluidParticleSystem &fluidParticleSystem, const Storm::IterationParameter &iterationParameter)
	{
		auto &dataField = reinterpret_cast<decltype(_data)*>(data)->find(pSystemId)->second;

		const std::vector<float> &masses = fluidParticleSystem.getMasses();
		std::vector<Storm::Vector3> &forces = fluidParticleSystem.getForces();
		std::vector<Storm::Vector3> &velocities = fluidParticleSystem.getVelocity();
		std::vector<Storm::Vector3> &positions = fluidParticleSystem.getPositions();
		std::vector<Storm::Vector3> &tmpPressureForces = fluidParticleSystem.getTemporaryPressureForces();

		std::atomic<bool> dirtyTmp = false;
		constexpr const float minForceDirtyEpsilon = 0.0001f;

		Storm::runParallel(dataField, [&](const Storm::IISPHSolverData &currentPData, const std::size_t currentPIndex)
		{
			const float currentPMass = masses[currentPIndex];
			forces[currentPIndex] = currentPData._predictedAcceleration * currentPMass;
			tmpPressureForces[currentPIndex] = (currentPData._predictedAcceleration - currentPData._nonPressureAcceleration) * currentPMass;

			// Euler integration
			Storm::Vector3 &currentPVelocity = velocities[currentPIndex];
			Storm::Vector3 &currentPPositions = positions[currentPIndex];

			currentPVelocity += currentPData._predictedAcceleration * iterationParameter._deltaTime;
			currentPPositions += currentPVelocity * iterationParameter._deltaTime;

			if (!dirtyTmp)
			{
				if (
					std::fabs(currentPVelocity.x()) > minForceDirtyEpsilon ||
					std::fabs(currentPVelocity.y()) > minForceDirtyEpsilon ||
					std::fabs(currentPVelocity.z()) > minForceDirtyEpsilon
					)
				{
					dirtyTmp = true;
				}
			}
		});

		fluidParticleSystem.setIsDirty(dirtyTmp);
	});

	// 7th : flush physics state (rigid bodies)
	simulMgr.flushPhysics(iterationParameter._deltaTime);
}
