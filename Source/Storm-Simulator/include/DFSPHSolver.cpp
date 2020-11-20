#include "DFSPHSolver.h"

#include "ParticleSystem.h"
#include "FluidParticleSystem.h"
#include "RigidBodyParticleSystem.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "GeneralSimulationData.h"
#include "FluidData.h"

#include "Kernel.h"

#include "DFSPHSolverData.h"

#define STORM_HIJACKED_TYPE Storm::DFSPHSolverData
#	include "VectHijack.h"
#undef STORM_HIJACKED_TYPE

#include "ThrowException.h"
#include "RunnerHelper.h"


namespace
{
#define STORM_SOLVER_NAMES_XMACRO \
	STORM_SOLVER_NAME("Density") \
	STORM_SOLVER_NAME("Divergence")

	using GUINames = std::remove_reference_t<Storm::PredictiveSolverHandler::SolversNames>;

	constexpr static GUINames g_solverIterationNames
	{
#define STORM_SOLVER_NAME(SolverName) STORM_TEXT(SolverName " solve iteration"),
		STORM_SOLVER_NAMES_XMACRO
#undef STORM_SOLVER_NAME
	};

	constexpr static GUINames g_solverErrorsNames
	{
#define STORM_SOLVER_NAME(SolverName) STORM_TEXT(SolverName " solve error"),
		STORM_SOLVER_NAMES_XMACRO
#undef STORM_SOLVER_NAME
	};

#undef STORM_SOLVER_NAMES_XMACRO
}


Storm::DFSPHSolver::DFSPHSolver(const float k_kernelLength, const Storm::ParticleSystemContainer &particleSystemsMap) :
	Storm::PredictiveSolverHandler{ g_solverIterationNames, g_solverErrorsNames }
{
	std::size_t totalParticleCount = 0;

	for (const auto &particleSystemPair : particleSystemsMap)
	{
		const Storm::ParticleSystem &pSystem = *particleSystemPair.second;
		if (pSystem.isFluids())
		{
			const Storm::VectorHijacker currentPSystemPCount{ pSystem.getParticleCount() };

			std::vector<Storm::DFSPHSolverData> &currentPSystemData = _data[particleSystemPair.first];
			currentPSystemData.reserve(currentPSystemPCount._newSize);
			Storm::setNumUninitialized_hijack(currentPSystemData, currentPSystemPCount);

			totalParticleCount += currentPSystemPCount._newSize;
		}
	}

	_totalParticleCountFl = static_cast<float>(totalParticleCount);

	// The only thing I won't do as the pseudo code reported in the original DFSPH article from J.Bender and D. Koshier (2017) is that I won't initialize k_dfsph coeff now
	// The reason is that neighborhoods weren't computed and the initialization rely on the actual position of particles and the neighborhood.
	// I could force an update here by manually initializing all particle systems but it is too dirty.
	// I prefer initializing everything when we'll compute the step because I'm sure that when we do, data structures will be correctly initialized.
}

Storm::DFSPHSolver::~DFSPHSolver() = default;

void Storm::DFSPHSolver::execute(Storm::ParticleSystemContainer &particleSystems, const float k_kernelLength, const float k_deltaTime)
{
	STORM_NOT_IMPLEMENTED;

	const Storm::SingletonHolder &singletonholder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonholder.getSingleton<Storm::IConfigManager>();

	const Storm::GeneralSimulationData &generalSimulData = configMgr.getGeneralSimulationData();

	const Storm::FluidData &fluidConfigData = configMgr.getFluidData();

	const float k_kernelZero = Storm::retrieveKernelZeroValue(generalSimulData._kernelMode);
	const Storm::RawKernelMethodDelegate rawKernel = Storm::retrieveRawKernelMethod(generalSimulData._kernelMode);
	const Storm::GradKernelMethodDelegate gradKernel = Storm::retrieveGradKernelMethod(generalSimulData._kernelMode);

	const float k_maxDensityError = generalSimulData._maxDensityError;

	const float deltaTimeSquared = k_deltaTime * k_deltaTime;
	const float k_kernelLengthSquared = k_kernelLength * k_kernelLength;

	// 1st : Compute the base density and init k_dfsph coeff
	for (auto &dataFieldPair : _data)
	{
		// Since data field was made from fluids particles only, no need to check if this is a fluid.
		Storm::FluidParticleSystem &fluidParticleSystem = static_cast<Storm::FluidParticleSystem &>(*particleSystems.find(dataFieldPair.first)->second);

		const float particleVolume = fluidParticleSystem.getParticleVolume();
		const float density0 = fluidParticleSystem.getRestDensity();
		const std::vector<Storm::ParticleNeighborhoodArray> &neighborhoodArrays = fluidParticleSystem.getNeighborhoodArrays();

		Storm::runParallel(fluidParticleSystem.getDensities(), [&](float &currentPDensity, const std::size_t currentPIndex)
		{
			// Density
			currentPDensity = particleVolume * k_kernelZero;

			//  ||Sum_j(mj x GradWij)||�
			Storm::Vector3 sumSquaredGradWij = Storm::Vector3::Zero();

			//  Sum_j(||mj x GradWij||�)
			float sumGradWijSquared = 0.f;

			// Those temp factor aren't equivalent : |a|� + |b|� + |c|� + ... |n|�  is not equal to (|a + b + c + ... + n|)� 

			const Storm::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoodArrays[currentPIndex];
			for (const Storm::NeighborParticleInfo &neighborInfo : currentPNeighborhood)
			{
				const float kernelValue_Wij = rawKernel(k_kernelLength, neighborInfo._vectToParticleNorm);
				Storm::Vector3 added = gradKernel(k_kernelLength, neighborInfo._positionDifferenceVector, neighborInfo._vectToParticleNorm);

				float deltaDensity;
				if (neighborInfo._isFluidParticle)
				{
					const Storm::FluidParticleSystem &neighborPSystemAsFluid = static_cast<const Storm::FluidParticleSystem &>(*neighborInfo._containingParticleSystem);
					deltaDensity = neighborPSystemAsFluid.getParticleVolume() * kernelValue_Wij;

					added *= neighborPSystemAsFluid.getParticleVolume();
				}
				else
				{
					const Storm::RigidBodyParticleSystem &neighborPSystemAsRb = static_cast<const Storm::RigidBodyParticleSystem &>(*neighborInfo._containingParticleSystem);
					deltaDensity = neighborPSystemAsRb.getVolumes()[neighborInfo._particleIndex] * kernelValue_Wij;

					added *= neighborPSystemAsRb.getVolumes()[neighborInfo._particleIndex];
				}

				currentPDensity += deltaDensity;

				sumSquaredGradWij += added;
				sumGradWijSquared += added.squaredNorm();
			}

			// Volume * density is mass...
			currentPDensity *= density0;

			Storm::DFSPHSolverData &currentPData = dataFieldPair.second[currentPIndex];
			currentPData._kCoeff = sumSquaredGradWij.squaredNorm() + sumGradWijSquared;
		});
	}

	// 2nd : Compute the non pressure forces (viscosity)
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
			std::vector<Storm::Vector3> &temporaryPViscoForces = fluidParticleSystem.getTemporaryViscosityForces();

			const std::vector<Storm::Vector3> &velocities = fluidParticleSystem.getVelocity();

			const float viscoPrecoeff = 0.01f * k_kernelLengthSquared;

			std::vector<Storm::DFSPHSolverData> &dataField = _data.find(particleSystemPair.first)->second;

			Storm::runParallel(fluidParticleSystem.getForces(), [&](Storm::Vector3 &currentPForce, const std::size_t currentPIndex)
			{
				const float currentPMass = masses[currentPIndex];
				const float currentPDensity = densities[currentPIndex];
				const Storm::Vector3 &vi = velocities[currentPIndex];

				const float restMassDensity = currentPMass * density0;

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
				Storm::DFSPHSolverData &currentPDataField = dataField[currentPIndex];

				currentPDataField._nonPressureAcceleration = currentPForce / currentPMass;
				currentPDataField._predictedAcceleration = currentPDataField._nonPressureAcceleration;
				currentPDataField._predictedVelocity = vi + currentPDataField._nonPressureAcceleration * k_deltaTime;
			});
		}
	}

	// 3rd : correct density error
	unsigned int densitySolvePredictionIter = 0;
	float averageDensityError = 0.f;

	do
	{
		// TODO

		++densitySolvePredictionIter;
	} while (densitySolvePredictionIter < generalSimulData._minPredictIteration || (densitySolvePredictionIter < generalSimulData._maxPredictIteration && averageDensityError > generalSimulData._maxDensityError));

	this->updateCurrentPredictionIter(densitySolvePredictionIter, generalSimulData._maxPredictIteration, averageDensityError, generalSimulData._maxDensityError, 0);



	// ?th : correct divergence error
	unsigned int divergenceSolvePredictionIter = 0;
	float averageDivergenceError = 0.f;

	do
	{
		// TODO

		++divergenceSolvePredictionIter;
	} while (divergenceSolvePredictionIter < generalSimulData._minPredictIteration || (divergenceSolvePredictionIter < generalSimulData._maxPredictIteration && averageDivergenceError > generalSimulData._maxDensityError));

	this->updateCurrentPredictionIter(divergenceSolvePredictionIter, generalSimulData._maxPredictIteration, averageDivergenceError, generalSimulData._maxDensityError, 1);


	this->transfertEndDataToSystems(particleSystems, &_data, [](void* data, const unsigned int pSystemId, Storm::FluidParticleSystem &fluidParticleSystem)
	{
		auto &dataField = reinterpret_cast<decltype(_data)*>(data)->find(pSystemId)->second;

		const std::vector<float> &masses = fluidParticleSystem.getMasses();
		std::vector<Storm::Vector3> &forces = fluidParticleSystem.getForces();
		std::vector<Storm::Vector3> &tmpPressureForces = fluidParticleSystem.getTemporaryPressureForces();

		Storm::runParallel(dataField, [&masses, &forces, &tmpPressureForces](const Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex)
		{
			const float currentPMass = masses[currentPIndex];
			forces[currentPIndex] = currentPData._predictedAcceleration * currentPMass;
			tmpPressureForces[currentPIndex] = (currentPData._predictedAcceleration - currentPData._nonPressureAcceleration) * currentPMass;
		});
	});
}
