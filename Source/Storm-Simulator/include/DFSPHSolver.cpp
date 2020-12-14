#include "DFSPHSolver.h"

#include "ParticleSystem.h"
#include "FluidParticleSystem.h"
#include "RigidBodyParticleSystem.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"
#include "SimulatorManager.h"

#include "GeneralSimulationData.h"
#include "FluidData.h"

#include "Kernel.h"

#include "DFSPHSolverData.h"

#include "IterationParameter.h"

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
#define STORM_SOLVER_NAME(SolverName) STORM_TEXT(SolverName " iteration"),
		STORM_SOLVER_NAMES_XMACRO
#undef STORM_SOLVER_NAME
	};

	constexpr static GUINames g_solverErrorsNames
	{
#define STORM_SOLVER_NAME(SolverName) STORM_TEXT(SolverName " error"),
		STORM_SOLVER_NAMES_XMACRO
#undef STORM_SOLVER_NAME
	};

#undef STORM_SOLVER_NAMES_XMACRO
}

#define STORM_REDO true

Storm::DFSPHSolver::DFSPHSolver(const float k_kernelLength, const Storm::ParticleSystemContainer &particleSystemsMap) :
	Storm::PredictiveSolverHandler{ g_solverIterationNames, g_solverErrorsNames }
{
	const Storm::SingletonHolder &singletonholder = Storm::SingletonHolder::instance();

	const Storm::IConfigManager &configMgr = singletonholder.getSingleton<Storm::IConfigManager>();
	const Storm::GeneralSimulationData &generalSimulData = configMgr.getGeneralSimulationData();
	const Storm::FluidData &fluidConfigData = configMgr.getFluidData();

	Storm::SimulatorManager &simulMgr = Storm::SimulatorManager::instance();
	simulMgr.refreshParticleNeighborhood();

	const float k_kernelZero = Storm::retrieveKernelZeroValue(generalSimulData._kernelMode);
	const Storm::RawKernelMethodDelegate rawKernel = Storm::retrieveRawKernelMethod(generalSimulData._kernelMode);
	const Storm::GradKernelMethodDelegate gradKernel = Storm::retrieveGradKernelMethod(generalSimulData._kernelMode);

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

#if STORM_REDO
			Storm::FluidParticleSystem &pSystemAsFluid = static_cast<Storm::FluidParticleSystem &>(*particleSystemPair.second);
			std::vector<float> &densities = pSystemAsFluid.getDensities();
			const std::vector<Storm::ParticleNeighborhoodArray> &neighborhoodArrays = pSystemAsFluid.getNeighborhoodArrays();
			const float particleVolume = pSystemAsFluid.getParticleVolume();
			const float density0 = pSystemAsFluid.getRestDensity();

			// 1st : Compute the base density and init k_dfsph coeff
			Storm::runParallel(currentPSystemData, [&](Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex)
			{
				float &currentPDensity = densities[currentPIndex];// Density
				currentPDensity = particleVolume * k_kernelZero;

				//  ||Sum_j(mj x GradWij)||²
				Storm::Vector3 sumSquaredGradWij = Storm::Vector3::Zero();

				//  Sum_j(||mj x GradWij||²)
				float sumGradWijSquared = 0.f;

				// Those temp factor aren't equivalent : |a|² + |b|² + |c|² + ... |n|²  is not equal to (|a + b + c + ... + n|)² 

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

				currentPData._kCoeff = sumSquaredGradWij.squaredNorm() + sumGradWijSquared;
				currentPData._predictedDensity = currentPDensity;
			});
#endif
		}
	}

	_totalParticleCountFl = static_cast<float>(totalParticleCount);
}

Storm::DFSPHSolver::~DFSPHSolver() = default;



#if STORM_REDO

void Storm::DFSPHSolver::execute(const Storm::IterationParameter &iterationParameter)
{
	const Storm::SingletonHolder &singletonholder = Storm::SingletonHolder::instance();

	const Storm::IConfigManager &configMgr = singletonholder.getSingleton<Storm::IConfigManager>();
	const Storm::GeneralSimulationData &generalSimulData = configMgr.getGeneralSimulationData();
	const Storm::FluidData &fluidConfigData = configMgr.getFluidData();

	Storm::SimulatorManager &simulMgr = Storm::SimulatorManager::instance();

	const float k_kernelZero = Storm::retrieveKernelZeroValue(generalSimulData._kernelMode);
	const Storm::RawKernelMethodDelegate rawKernel = Storm::retrieveRawKernelMethod(generalSimulData._kernelMode);
	const Storm::GradKernelMethodDelegate gradKernel = Storm::retrieveGradKernelMethod(generalSimulData._kernelMode);

	const float k_maxDensityError = generalSimulData._maxDensityError;

	const float deltaTimeSquared = iterationParameter._deltaTime * iterationParameter._deltaTime;
	const float k_kernelLengthSquared = iterationParameter._kernelLength * iterationParameter._kernelLength;

	Storm::ParticleSystemContainer &particleSystems = *iterationParameter._particleSystems;

	// 1st : Initialize iteration
	simulMgr.advanceBlowersTime(iterationParameter._deltaTime);
	simulMgr.refreshParticleNeighborhood();
	simulMgr.subIterationStart();

	// 2nd : Compute the non pressure forces (viscosity)
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

			std::vector<Storm::DFSPHSolverData> &dataField = _data.find(particleSystemPair.first)->second;

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
				currentPDataField._predictedVelocity = vi + currentPDataField._nonPressureAcceleration * iterationParameter._deltaTime;
			});
		}
	}

	// TODO : CFL here



	// 3rd : correct density error
	unsigned int densitySolvePredictionIter = 0;
	float averageDensityError;
	std::atomic<float> avgDensityAtom;

	const float maxDensityError = generalSimulData._maxDensityError;
	const unsigned int minDensityIteration = generalSimulData._minPredictIteration;
	const unsigned int maxDensityIteration = generalSimulData._maxPredictIteration;

	do 
	{
		averageDensityError = 0.f;

		// 3-1 : predict density
		for (auto &dataFieldPair : _data)
		{
			// Since data field was made from fluids particles only, no need to check if this is a fluid.
			Storm::FluidParticleSystem &fluidParticleSystem = static_cast<Storm::FluidParticleSystem &>(*particleSystems.find(dataFieldPair.first)->second);

			const float density0 = fluidParticleSystem.getRestDensity();
			const float particleVolume = fluidParticleSystem.getParticleVolume();
			const std::vector<Storm::ParticleNeighborhoodArray> &neighborhoodArrays = fluidParticleSystem.getNeighborhoodArrays();
			const std::vector<float> &densities = fluidParticleSystem.getDensities();

			avgDensityAtom = 0.f;

			Storm::runParallel(dataFieldPair.second, [&](Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex)
			{
				const Storm::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoodArrays[currentPIndex];

				const std::vector<Storm::DFSPHSolverData>* neighborDataArray = &dataFieldPair.second;
				const Storm::FluidParticleSystem* lastNeighborFluidSystem = &fluidParticleSystem;

				const Storm::Vector3 &currentPPredictedVelocity = currentPData._predictedVelocity;

				currentPData._predictedDensity = 0.f;
				for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
				{
					const Storm::Vector3 gradWij = gradKernel(iterationParameter._kernelLength, neighbor._positionDifferenceVector, neighbor._vectToParticleNorm);

					if (neighbor._isFluidParticle)
					{
						if (neighbor._containingParticleSystem != lastNeighborFluidSystem)
						{
							lastNeighborFluidSystem = static_cast<const Storm::FluidParticleSystem*>(neighbor._containingParticleSystem);
							neighborDataArray = &_data.find(lastNeighborFluidSystem->getId())->second;
						}

						const Storm::DFSPHSolverData &neighborData = (*neighborDataArray)[neighbor._particleIndex];

						currentPData._predictedDensity += lastNeighborFluidSystem->getMasses()[neighbor._particleIndex] * (currentPPredictedVelocity - neighborData._predictedVelocity).dot(gradWij);
					}
					else
					{
						const Storm::RigidBodyParticleSystem &neighborPSystemAsRb = static_cast<const Storm::RigidBodyParticleSystem &>(*neighbor._containingParticleSystem);
						currentPData._predictedDensity += neighborPSystemAsRb.getVolumes()[neighbor._particleIndex] * particleVolume * (currentPPredictedVelocity - neighborPSystemAsRb.getVelocity()[neighbor._particleIndex]).dot(gradWij);
					}
				}

				currentPData._predictedDensity *= iterationParameter._deltaTime;
				currentPData._predictedDensity += densities[currentPIndex];

				currentPData._predictedDensity = std::max(currentPData._predictedDensity, 0.f);

				const float deltaError = (currentPData._predictedDensity - density0) / _totalParticleCountFl;
				avgDensityAtom += deltaError;
			});

			averageDensityError += avgDensityAtom / density0;
		}

		// 3-2 : adapt velocity
		for (auto &dataFieldPair : _data)
		{
			// Since data field was made from fluids particles only, no need to check if this is a fluid.
			Storm::FluidParticleSystem &fluidParticleSystem = static_cast<Storm::FluidParticleSystem &>(*particleSystems.find(dataFieldPair.first)->second);

			const float density0 = fluidParticleSystem.getRestDensity();
			const float particleVolume = fluidParticleSystem.getParticleVolume();
			const std::vector<Storm::ParticleNeighborhoodArray> &neighborhoodArrays = fluidParticleSystem.getNeighborhoodArrays();
			const std::vector<float> &masses = fluidParticleSystem.getMasses();
			const std::vector<float> &densities = fluidParticleSystem.getDensities();

			Storm::runParallel(dataFieldPair.second, [&](Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex)
			{
				const float currentPDensity = densities[currentPIndex];
				const float ki = (currentPData._predictedDensity - density0) / (deltaTimeSquared * density0) * currentPData._kCoeff;

				const std::vector<Storm::DFSPHSolverData>* neighborDataArray = &dataFieldPair.second;
				const Storm::FluidParticleSystem* lastNeighborFluidSystem = &fluidParticleSystem;

				const Storm::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoodArrays[currentPIndex];
				for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
				{
					const Storm::Vector3 gradWij = gradKernel(iterationParameter._kernelLength, neighbor._positionDifferenceVector, neighbor._vectToParticleNorm);

					float deltaCoeff;
					if (neighbor._isFluidParticle)
					{
						if (neighbor._containingParticleSystem != lastNeighborFluidSystem)
						{
							lastNeighborFluidSystem = static_cast<const Storm::FluidParticleSystem*>(neighbor._containingParticleSystem);
							neighborDataArray = &_data.find(lastNeighborFluidSystem->getId())->second;
						}

						const Storm::DFSPHSolverData &neighborData = (*neighborDataArray)[neighbor._particleIndex];

						const float kj = (neighborData._predictedDensity - lastNeighborFluidSystem->getRestDensity()) / deltaTimeSquared * neighborData._kCoeff;
						deltaCoeff = lastNeighborFluidSystem->getMasses()[neighbor._particleIndex] * (ki + kj / lastNeighborFluidSystem->getDensities()[neighbor._particleIndex]);
					}
					else
					{
						const Storm::RigidBodyParticleSystem &neighborPSystemAsRb = static_cast<const Storm::RigidBodyParticleSystem &>(*neighbor._containingParticleSystem);
						deltaCoeff = neighborPSystemAsRb.getVolumes()[neighbor._particleIndex] * ki;
					}

					deltaCoeff *= iterationParameter._deltaTime;

					currentPData._predictedVelocity -= deltaCoeff * gradWij;
				}
			});
		}

		++densitySolvePredictionIter;

	} while (densitySolvePredictionIter < minDensityIteration || (densitySolvePredictionIter < maxDensityIteration && averageDensityError > maxDensityError));
	this->updateCurrentPredictionIter(densitySolvePredictionIter, maxDensityIteration, averageDensityError, maxDensityError, 0);


}


#else
void Storm::DFSPHSolver::execute(const Storm::IterationParameter &iterationParameter)
{
	//STORM_NOT_IMPLEMENTED;

	const Storm::SingletonHolder &singletonholder = Storm::SingletonHolder::instance();

	const Storm::IConfigManager &configMgr = singletonholder.getSingleton<Storm::IConfigManager>();
	const Storm::GeneralSimulationData &generalSimulData = configMgr.getGeneralSimulationData();
	const Storm::FluidData &fluidConfigData = configMgr.getFluidData();

	Storm::SimulatorManager &simulMgr = Storm::SimulatorManager::instance();

	const float k_kernelZero = Storm::retrieveKernelZeroValue(generalSimulData._kernelMode);
	const Storm::RawKernelMethodDelegate rawKernel = Storm::retrieveRawKernelMethod(generalSimulData._kernelMode);
	const Storm::GradKernelMethodDelegate gradKernel = Storm::retrieveGradKernelMethod(generalSimulData._kernelMode);

	const float k_maxDensityError = generalSimulData._maxDensityError;

	const float deltaTimeSquared = iterationParameter._deltaTime * iterationParameter._deltaTime;
	const float k_kernelLengthSquared = iterationParameter._kernelLength * iterationParameter._kernelLength;

	Storm::ParticleSystemContainer &particleSystems = *iterationParameter._particleSystems;

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

			//  ||Sum_j(mj x GradWij)||²
			Storm::Vector3 sumSquaredGradWij = Storm::Vector3::Zero();

			//  Sum_j(||mj x GradWij||²)
			float sumGradWijSquared = 0.f;

			// Those temp factor aren't equivalent : |a|² + |b|² + |c|² + ... |n|²  is not equal to (|a + b + c + ... + n|)² 

			const Storm::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoodArrays[currentPIndex];
			for (const Storm::NeighborParticleInfo &neighborInfo : currentPNeighborhood)
			{
				const float kernelValue_Wij = rawKernel(iterationParameter._kernelLength, neighborInfo._vectToParticleNorm);
				Storm::Vector3 added = gradKernel(iterationParameter._kernelLength, neighborInfo._positionDifferenceVector, neighborInfo._vectToParticleNorm);

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
			currentPData._predictedDensity = currentPDensity;
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
					const Storm::Vector3 gradKernel_NablaWij = gradKernel(iterationParameter._kernelLength, neighbor._positionDifferenceVector, neighbor._vectToParticleNorm);

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
				currentPDataField._predictedVelocity = vi + currentPDataField._nonPressureAcceleration * iterationParameter._deltaTime;
			});
		}
	}

	// 3rd : correct density error
	unsigned int densitySolvePredictionIter = 0;
	float averageDensityError = 0.f;
	std::atomic<float> avgDensityAtom;

	const float maxDensityError = generalSimulData._maxDensityError;
	const unsigned int minDensityIteration = generalSimulData._minPredictIteration;
	const unsigned int maxDensityIteration = generalSimulData._maxPredictIteration;

	do
	{
		// 3-1 : Compute predicted density
		for (auto &dataFieldPair : _data)
		{
			// Since data field was made from fluids particles only, no need to check if this is a fluid.
			Storm::FluidParticleSystem &fluidParticleSystem = static_cast<Storm::FluidParticleSystem &>(*particleSystems.find(dataFieldPair.first)->second);
			std::vector<float> &densities = fluidParticleSystem.getDensities();
			const std::vector<Storm::ParticleNeighborhoodArray> &neighborhoodArrays = fluidParticleSystem.getNeighborhoodArrays();

			Storm::runParallel(dataFieldPair.second, [&](Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex) 
			{
				float &currentPLastDensity = densities[currentPIndex];
				const Storm::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoodArrays[currentPIndex];

				const std::vector<Storm::DFSPHSolverData>* neighborDataArray = &dataFieldPair.second;
				const Storm::FluidParticleSystem* lastNeighborFluidSystem = &fluidParticleSystem;

				const Storm::Vector3 &currentPPredictedVelocity = currentPData._predictedVelocity;

				currentPData._predictedDensity = 0.f;

				for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
				{
					const Storm::Vector3 gradWij = gradKernel(iterationParameter._kernelLength, neighbor._positionDifferenceVector, neighbor._vectToParticleNorm);

					if (neighbor._isFluidParticle)
					{
						if (neighbor._containingParticleSystem != lastNeighborFluidSystem)
						{
							lastNeighborFluidSystem = static_cast<const Storm::FluidParticleSystem*>(neighbor._containingParticleSystem);
							neighborDataArray = &_data.find(lastNeighborFluidSystem->getId())->second;
						}

						const Storm::DFSPHSolverData &neighborData = (*neighborDataArray)[neighbor._particleIndex];
						const Storm::Vector3 &neighborPredictedVelocity = neighborData._predictedVelocity;

						currentPData._predictedDensity += lastNeighborFluidSystem->getParticleVolume() * (currentPPredictedVelocity - neighborPredictedVelocity).dot(gradWij);
					}
					else
					{
						const Storm::RigidBodyParticleSystem &lastNeighborPSystemAsRb = static_cast<const Storm::RigidBodyParticleSystem &>(*neighbor._containingParticleSystem);

						const Storm::Vector3 &neighborPredictedVelocity = lastNeighborPSystemAsRb.getVelocity()[currentPIndex];

						currentPData._predictedDensity += lastNeighborPSystemAsRb.getVolumes()[currentPIndex] * (currentPPredictedVelocity - neighborPredictedVelocity).dot(gradWij);
					}
				}

				currentPData._predictedDensity *= iterationParameter._deltaTime;
				currentPData._predictedDensity += currentPLastDensity;
			});
		}

		avgDensityAtom = 0.f;

		// 3-2 : Compute predicted velocity from predicted density + init for next time
		for (auto &dataFieldPair : _data)
		{
			// Since data field was made from fluids particles only, no need to check if this is a fluid.
			Storm::FluidParticleSystem &fluidParticleSystem = static_cast<Storm::FluidParticleSystem &>(*particleSystems.find(dataFieldPair.first)->second);
			std::vector<float> &densities = fluidParticleSystem.getDensities();
			const std::vector<Storm::ParticleNeighborhoodArray> &neighborhoodArrays = fluidParticleSystem.getNeighborhoodArrays();

			const float density0 = fluidParticleSystem.getRestDensity();

			Storm::runParallel(dataFieldPair.second, [&](Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex)
			{
				float &currentPLastDensity = densities[currentPIndex];
				const Storm::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoodArrays[currentPIndex];

				const std::vector<Storm::DFSPHSolverData>* neighborDataArray = &dataFieldPair.second;
				const Storm::FluidParticleSystem* lastNeighborFluidSystem = &fluidParticleSystem;

				currentPLastDensity = currentPData._predictedDensity;

				const float currentPRelativeDensity = currentPData._predictedDensity - density0;

				// Divide by rest density to make it uniform (many fluids have their own rest density, therefore we should remove the rest density dependency).
				const float avgDensityAdded = currentPRelativeDensity / (_totalParticleCountFl * density0);
				avgDensityAtom += avgDensityAdded;

				const float currentPPredictedPressure = currentPRelativeDensity * currentPData._kCoeff;
				const float currentPPredictedPressureCoeff = currentPPredictedPressure / (currentPData._predictedDensity * currentPData._predictedDensity);

				Storm::Vector3 sumTmp = Storm::Vector3::Zero();

				for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
				{
					const Storm::Vector3 gradWij = gradKernel(iterationParameter._kernelLength, neighbor._positionDifferenceVector, neighbor._vectToParticleNorm);

					float coeff;
					if (neighbor._isFluidParticle)
					{
						if (neighbor._containingParticleSystem != lastNeighborFluidSystem)
						{
							lastNeighborFluidSystem = static_cast<const Storm::FluidParticleSystem*>(neighbor._containingParticleSystem);
							neighborDataArray = &_data.find(lastNeighborFluidSystem->getId())->second;
						}

						const Storm::DFSPHSolverData &neighborData = (*neighborDataArray)[neighbor._particleIndex];

						const float neighborPPredictedPressure = (neighborData._predictedDensity - density0) * neighborData._kCoeff;
						const float neighborPPredictedPressureCoeff = neighborPPredictedPressure / (neighborData._predictedDensity * neighborData._predictedDensity);

						coeff = lastNeighborFluidSystem->getMasses()[neighbor._particleIndex] * (currentPPredictedPressureCoeff + neighborPPredictedPressureCoeff);
					}
					else
					{
						const Storm::RigidBodyParticleSystem &lastNeighborPSystemAsRb = static_cast<const Storm::RigidBodyParticleSystem &>(*neighbor._containingParticleSystem);

						// ???
						coeff = lastNeighborPSystemAsRb.getVolumes()[neighbor._particleIndex] * density0 * 2.f * currentPPredictedPressureCoeff;
					}

					sumTmp += coeff * gradWij;
				}

				sumTmp /= iterationParameter._deltaTime;

				currentPData._predictedVelocity -= sumTmp;
			});
		}

		averageDensityError = std::fabs(avgDensityAtom);
		++densitySolvePredictionIter;

	} while (densitySolvePredictionIter < minDensityIteration || (densitySolvePredictionIter < maxDensityIteration && averageDensityError > maxDensityError));

	this->updateCurrentPredictionIter(densitySolvePredictionIter, maxDensityIteration, averageDensityError, maxDensityError, 0);

	// 3-3 : Apply position changes.
	for (auto &dataFieldPair : _data)
	{
		// Since data field was made from fluids particles only, no need to check if this is a fluid.
		Storm::FluidParticleSystem &fluidParticleSystem = static_cast<Storm::FluidParticleSystem &>(*particleSystems.find(dataFieldPair.first)->second);
		std::vector<Storm::Vector3> &positions = fluidParticleSystem.getPositions();

		Storm::runParallel(dataFieldPair.second, [&](Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex)
		{
			positions[currentPIndex] += iterationParameter._deltaTime * currentPData._predictedVelocity;

			currentPData._finalPredictedVelocity = currentPData._predictedVelocity;
		});
	}

	//4th : Refresh neighborhoods 
	simulMgr.refreshParticleNeighborhood();

	// 5th : correct divergence error
	unsigned int divergenceSolvePredictionIter = 0;
	float averageDivergenceError;

	const float maxDivergenceError = generalSimulData._maxDensityError;
	const unsigned int minDivergenceIteration = generalSimulData._minPredictIteration;
	const unsigned int maxDivergenceIteration = generalSimulData._maxPredictIteration;

	do
	{
		for (auto &dataFieldPair : _data)
		{
			// Since data field was made from fluids particles only, no need to check if this is a fluid.
			Storm::FluidParticleSystem &fluidParticleSystem = static_cast<Storm::FluidParticleSystem &>(*particleSystems.find(dataFieldPair.first)->second);

			const std::vector<Storm::ParticleNeighborhoodArray> &neighborhoodArrays = fluidParticleSystem.getNeighborhoodArrays();
			const std::vector<float> &masses = fluidParticleSystem.getMasses();
			const std::vector<Storm::Vector3> &positions = fluidParticleSystem.getPositions();

			const float density0 = fluidParticleSystem.getRestDensity();

			constexpr const float k_epsilon = 0.00001f;

			Storm::runParallel(dataFieldPair.second, [&](Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex)
			{
				const Storm::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoodArrays[currentPIndex];
				const float currentPMass = masses[currentPIndex];

				const std::vector<Storm::DFSPHSolverData>* neighborDataArray = &dataFieldPair.second;
				const Storm::FluidParticleSystem* lastNeighborFluidSystem = &fluidParticleSystem;

				for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
				{
					const Storm::Vector3 gradWij = gradKernel(iterationParameter._kernelLength, neighbor._positionDifferenceVector, neighbor._vectToParticleNorm);
					if (neighbor._isFluidParticle)
					{
						if (neighbor._containingParticleSystem != lastNeighborFluidSystem)
						{
							lastNeighborFluidSystem = static_cast<const Storm::FluidParticleSystem*>(neighbor._containingParticleSystem);
							neighborDataArray = &_data.find(lastNeighborFluidSystem->getId())->second;
						}

						const Storm::DFSPHSolverData &neighborData = (*neighborDataArray)[neighbor._particleIndex];
						const float kNeighborCoeff = currentPData._kCoeff + (lastNeighborFluidSystem->getRestDensity() / density0) * neighborData._kCoeff;

						currentPData._finalPredictedVelocity += (iterationParameter._deltaTime * kNeighborCoeff) * gradWij;
					}
					else
					{
						const Storm::Vector3 pressureAccel = currentPData._kCoeff * gradWij;
						const Storm::Vector3 velocityChange = iterationParameter._deltaTime * pressureAccel;
						currentPData._finalPredictedVelocity += velocityChange;

						Storm::RigidBodyParticleSystem &otherPSystemAsRb = static_cast<Storm::RigidBodyParticleSystem &>(*neighbor._containingParticleSystem);
						if (!otherPSystemAsRb.isStatic())
						{
							const Storm::Vector3 pressureForceAdded = currentPMass * pressureAccel;
							Storm::Vector3 &neighborPressureForce = otherPSystemAsRb.getTemporaryPressureForces()[neighbor._particleIndex];

							std::lock_guard<std::mutex> lock{ otherPSystemAsRb._mutex };
							neighborPressureForce -= pressureForceAdded;
						}
					}
				}
			});
		}

		averageDivergenceError = 0.f;
		std::atomic<float> averageSumDivergenceErrorAtom;

		for (auto &dataFieldPair : _data)
		{
			// Since data field was made from fluids particles only, no need to check if this is a fluid.
			Storm::FluidParticleSystem &fluidParticleSystem = static_cast<Storm::FluidParticleSystem &>(*particleSystems.find(dataFieldPair.first)->second);

			const std::vector<Storm::ParticleNeighborhoodArray> &neighborhoodArrays = fluidParticleSystem.getNeighborhoodArrays();

			averageSumDivergenceErrorAtom = 0.f;

			Storm::runParallel(dataFieldPair.second, [&](Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex)
			{
				const Storm::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoodArrays[currentPIndex];

				const std::vector<Storm::DFSPHSolverData>* neighborDataArray = &dataFieldPair.second;
				const Storm::FluidParticleSystem* lastNeighborFluidSystem = &fluidParticleSystem;

				float currentPDeltaDivergenceError = 0.f;

				for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
				{
					const Storm::Vector3 gradWij = gradKernel(iterationParameter._kernelLength, neighbor._positionDifferenceVector, neighbor._vectToParticleNorm);
					if (neighbor._isFluidParticle)
					{
						if (neighbor._containingParticleSystem != lastNeighborFluidSystem)
						{
							lastNeighborFluidSystem = static_cast<const Storm::FluidParticleSystem*>(neighbor._containingParticleSystem);
							neighborDataArray = &_data.find(lastNeighborFluidSystem->getId())->second;
						}

						const Storm::DFSPHSolverData &neighborData = (*neighborDataArray)[neighbor._particleIndex];

						const Storm::Vector3 velocityDiff = currentPData._finalPredictedVelocity - neighborData._finalPredictedVelocity;
						currentPDeltaDivergenceError += velocityDiff.dot(gradWij) * lastNeighborFluidSystem->getParticleVolume();
					}
					else
					{
						const Storm::RigidBodyParticleSystem &neighborPSystemAsRb = static_cast<const Storm::RigidBodyParticleSystem &>(*neighbor._containingParticleSystem);

						const Storm::Vector3 velocityDiff = currentPData._finalPredictedVelocity - neighborPSystemAsRb.getVelocity()[neighbor._particleIndex];
						currentPDeltaDivergenceError += velocityDiff.dot(gradWij) * neighborPSystemAsRb.getVolumes()[neighbor._particleIndex];
					}
				}

				if (std::fabs(currentPDeltaDivergenceError) > 0.000001f)
				{
					currentPDeltaDivergenceError *= iterationParameter._deltaTime;
					currentPDeltaDivergenceError = currentPData._predictedDensity / currentPDeltaDivergenceError;

					averageSumDivergenceErrorAtom += currentPDeltaDivergenceError;
				}
			});

			const float coeff = iterationParameter._deltaTime * fluidParticleSystem.getRestDensity() / _totalParticleCountFl;
			averageDivergenceError += coeff * averageSumDivergenceErrorAtom;
		}

		++divergenceSolvePredictionIter;
	} while (divergenceSolvePredictionIter < minDivergenceIteration || (divergenceSolvePredictionIter < maxDivergenceIteration && averageDivergenceError > maxDivergenceError));

	this->updateCurrentPredictionIter(divergenceSolvePredictionIter, maxDivergenceIteration, averageDivergenceError, maxDivergenceError, 1);

	// 6th update positions
	this->transfertEndDataToSystems(particleSystems, iterationParameter, &_data, [](void* data, const unsigned int pSystemId, Storm::FluidParticleSystem &fluidParticleSystem, const Storm::IterationParameter &iterationParameter)
	{
		auto &dataField = reinterpret_cast<decltype(_data)*>(data)->find(pSystemId)->second;

		const std::vector<float> &masses = fluidParticleSystem.getMasses();
		std::vector<Storm::Vector3> &forces = fluidParticleSystem.getForces();
		std::vector<Storm::Vector3> &tmpPressureForces = fluidParticleSystem.getTemporaryPressureForces();
		std::vector<Storm::Vector3> &velocities = fluidParticleSystem.getVelocity();
		std::vector<Storm::Vector3> &positions = fluidParticleSystem.getPositions();

		Storm::runParallel(dataField, [&](const Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex)
		{
			const Storm::Vector3 currentPPressureAccel = currentPData._predictedAcceleration - currentPData._nonPressureAcceleration;
			const Storm::Vector3 currentPPressureAddedVelocity = currentPPressureAccel * iterationParameter._deltaTime;

			const float currentPMass = masses[currentPIndex];
			forces[currentPIndex] = currentPData._predictedAcceleration * currentPMass;
			tmpPressureForces[currentPIndex] = currentPPressureAccel * currentPMass;
			velocities[currentPIndex] = currentPData._finalPredictedVelocity;
			positions[currentPIndex] += currentPData._finalPredictedVelocity * iterationParameter._deltaTime;
		});
	});

	// 7th : flush physics state (rigid bodies)
	simulMgr.flushPhysics(iterationParameter._deltaTime);
}
#endif
