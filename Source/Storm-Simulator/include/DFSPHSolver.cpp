//
// Note :
// This implementation was taken from the implementation J.Bender & Al. did inside SplishSplash
// and re adapted to suit how my custom engine works.
//
//

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

#include "RunnerHelper.h"


namespace
{
#define STORM_SOLVER_NAMES_XMACRO \
	STORM_SOLVER_NAME("Divergence") \
	STORM_SOLVER_NAME("Pressure")

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


Storm::DFSPHSolver::DFSPHSolver(const float k_kernelLength, const Storm::ParticleSystemContainer &particleSystemsMap) :
	Storm::PredictiveSolverHandler{ g_solverIterationNames, g_solverErrorsNames },
	_enableDivergenceSolve{ true }
{
	Storm::SimulatorManager &simulMgr = Storm::SimulatorManager::instance();
	simulMgr.refreshParticleNeighborhood();

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

			Storm::runParallel(currentPSystemData, [](Storm::DFSPHSolverData &currentPData)
			{
				currentPData._predictedVelocity.setZero();
				currentPData._nonPressureAcceleration.setZero();
				currentPData._kCoeff = 0.f;
				currentPData._predictedDensity = 0.f;
			});

			totalParticleCount += currentPSystemPCount._newSize;
		}
	}

	_totalParticleCountFl = static_cast<float>(totalParticleCount);
}

Storm::DFSPHSolver::~DFSPHSolver() = default;

void Storm::DFSPHSolver::execute(const Storm::IterationParameter &iterationParameter)
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

	const Storm::GeneralSimulationData &generalSimulConfig = configMgr.getGeneralSimulationData();
	const Storm::FluidData &fluidConfigData = configMgr.getFluidData();

	Storm::SimulatorManager &simulMgr = Storm::SimulatorManager::instance();

	const float k_kernelZero = Storm::retrieveKernelZeroValue(generalSimulConfig._kernelMode);

	const float k_kernelLengthSquared = iterationParameter._kernelLength * iterationParameter._kernelLength;

	Storm::ParticleSystemContainer &particleSystems = *iterationParameter._particleSystems;

	// 1st : Initialize iteration
	simulMgr.advanceBlowersTime(iterationParameter._deltaTime);
	simulMgr.refreshParticleNeighborhood();
	simulMgr.subIterationStart();

	// 2nd : refresh particle neighborhood
	simulMgr.refreshParticleNeighborhood();

	// 3rd : Compute the base density
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
					float deltaDensity;
					if (neighbor._isFluidParticle)
					{
						deltaDensity = static_cast<Storm::FluidParticleSystem*>(neighbor._containingParticleSystem)->getParticleVolume() * neighbor._Wij;
					}
					else
					{
						deltaDensity = static_cast<Storm::RigidBodyParticleSystem*>(neighbor._containingParticleSystem)->getVolumes()[neighbor._particleIndex] * neighbor._Wij;
					}
					currentPDensity += deltaDensity;
				}

				// Volume * density is mass...
				currentPDensity *= density0;
			});
		}
	}

	// 4th : Compute k_dfsph coeff
	for (auto &dataFieldPair : _data)
	{
		this->computeDFSPHFactor(
			iterationParameter,
			static_cast<Storm::FluidParticleSystem &>(*particleSystems.find(dataFieldPair.first)->second), // Since data field was made from fluid particles, no need to check.
			dataFieldPair.second
		);
	}

	// 5th : Divergence solve
	unsigned int iterationV;
	float averageErrorV;
	if (_enableDivergenceSolve)
	{
		this->divergenceSolve(iterationParameter, iterationV, averageErrorV);
	}
	else
	{
		iterationV = 0;
		averageErrorV = 0.f;
	}

	this->updateCurrentPredictionIter(iterationV, generalSimulConfig._maxPredictIteration, averageErrorV, generalSimulConfig._maxDensityError, 0);

	// 7th : Compute the non pressure forces (viscosity)
	// Compute accelerations: a(t)
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

			Storm::DFSPHSolver::DFSPHSolverDataArray &dataField = _data.find(particleSystemPair.first)->second;

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
						viscosityComponent = (viscoGlobalCoeff * fluidConfigData._dynamicViscosity * neighborMass / neighborRawDensity) * neighbor._gradWij;
					}
					else
					{
						const Storm::RigidBodyParticleSystem* neighborPSystemAsBoundary = static_cast<Storm::RigidBodyParticleSystem*>(neighbor._containingParticleSystem);

						const float neighborVolume = neighborPSystemAsBoundary->getVolumes()[neighbor._particleIndex];
						const float rbViscosity = neighborPSystemAsBoundary->getViscosity();

						// Viscosity
						if (rbViscosity > 0.f)
						{
							viscosityComponent = (viscoGlobalCoeff * rbViscosity * neighborVolume * density0 / currentPDensity) * neighbor._gradWij;
						}
						else
						{
							viscosityComponent = Storm::Vector3::Zero();
						}

						// Mirror the force on the boundary solid following the 3rd newton law
						if (!neighborPSystemAsBoundary->isStatic())
						{
							Storm::Vector3 &boundaryNeighborForce = neighbor._containingParticleSystem->getForces()[neighbor._particleIndex];
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
				Storm::DFSPHSolverData &currentPDataField = dataField[currentPIndex];

				currentPDataField._nonPressureAcceleration = currentPForce / currentPMass;
				currentPDataField._predictedVelocity = vi + currentPDataField._nonPressureAcceleration * iterationParameter._deltaTime;

				// Note : Maybe we should also compute a prediction of the position ?
			});
		}
	}

	// 8th : TODO : CFL
	// ...

	// 9th : Solve pressure
	unsigned int iteration;
	float averageError;
	this->pressureSolve(iterationParameter, iteration, averageError);

	this->updateCurrentPredictionIter(iteration, generalSimulConfig._maxPredictIteration, averageError, generalSimulConfig._maxDensityError, 1);

	///////////////////////////////////////////////////

	// 10th : compute final positions
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

		Storm::runParallel(dataField, [&](const Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex)
		{
			// Euler integration
			Storm::Vector3 &currentPVelocity = velocities[currentPIndex];
			Storm::Vector3 &currentPPositions = positions[currentPIndex];

			const float currentPMass = masses[currentPIndex];

			// Since all pressure computation was done on currentPData._predictedVelocity, then the difference is the velocity delta that was added from the pressure effect.
			const Storm::Vector3 pressureVelocityAccel = (currentPData._predictedVelocity - currentPVelocity) / iterationParameter._deltaTime;
			tmpPressureForces[currentPIndex] = pressureVelocityAccel * currentPMass;

			const Storm::Vector3 totalAccel = currentPData._nonPressureAcceleration + pressureVelocityAccel;
			forces[currentPIndex] = totalAccel * currentPMass;

			currentPVelocity = currentPData._predictedVelocity;
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

	// 11th : flush physics state (rigid bodies)
	simulMgr.flushPhysics(iterationParameter._deltaTime);
}

void Storm::DFSPHSolver::divergenceSolve(const Storm::IterationParameter &iterationParameter, unsigned int &outIteration, float &outAverageError)
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

	const Storm::GeneralSimulationData &generalSimulConfig = configMgr.getGeneralSimulationData();

	Storm::SimulatorManager &simulMgr = Storm::SimulatorManager::instance();

	Storm::ParticleSystemContainer &particleSystems = *iterationParameter._particleSystems;

	//////////////////////////////////////////////////////////////////////////
	// Init parameters
	//////////////////////////////////////////////////////////////////////////

	const float invertDeltaTime = 1.f / iterationParameter._deltaTime;

	const unsigned int minIter = generalSimulConfig._minPredictIteration;
	const unsigned int maxIter = generalSimulConfig._maxPredictIteration;
	const float maxError = generalSimulConfig._maxDensityError;

	//////////////////////////////////////////////////////////////////////////
	// Compute velocity of density change
	//////////////////////////////////////////////////////////////////////////
	for (auto &dataFieldPair : _data)
	{
		// Since data field was made from fluid particles, no need to check.
		Storm::FluidParticleSystem &fluidPSystem = static_cast<Storm::FluidParticleSystem &>(*particleSystems.find(dataFieldPair.first)->second);
		Storm::runParallel(dataFieldPair.second, [this, &iterationParameter, &fluidPSystem, &dataFieldPair, invertDeltaTime](Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex)
		{
			this->computeDensityChange(iterationParameter, fluidPSystem, &dataFieldPair.second, currentPData, currentPIndex);
			currentPData._kCoeff *= invertDeltaTime;
		});
	}

	outIteration = 0;

	//////////////////////////////////////////////////////////////////////////
	// Start solver
	//////////////////////////////////////////////////////////////////////////

	bool chk;

	constexpr const float k_epsilon = 0.00001f;

	do
	{
		outAverageError = 0.f;

		chk = true;

		std::atomic<float> avgDensityErrAtom;
		for (auto &dataFieldPair : _data)
		{
			// Since data field was made from fluid particles, no need to check.
			Storm::FluidParticleSystem &fluidPSystem = static_cast<Storm::FluidParticleSystem &>(*particleSystems.find(dataFieldPair.first)->second);
			
			const std::vector<float> &masses = fluidPSystem.getMasses();
			const std::vector<Storm::ParticleNeighborhoodArray> &neighborhoodArrays = fluidPSystem.getNeighborhoodArrays();

			const float density0 = fluidPSystem.getRestDensity();
			avgDensityErrAtom = 0.f;

			float density_error = 0.f;

			Storm::runParallel(dataFieldPair.second, [&](Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex)
			{
				const float currentPMass = masses[currentPIndex];

				//////////////////////////////////////////////////////////////////////////
				// Evaluate rhs
				//////////////////////////////////////////////////////////////////////////
				const float b_i = currentPData._predictedDensity;
				const float ki = b_i * currentPData._kCoeff;

				Storm::Vector3 &v_i = currentPData._predictedVelocity;

				const Storm::DFSPHSolver::DFSPHSolverDataArray* neighborDataArray = &dataFieldPair.second;
				const Storm::FluidParticleSystem* lastNeighborFluidSystem = &fluidPSystem;

				//////////////////////////////////////////////////////////////////////////
				// Perform Jacobi iteration over all blocks
				//////////////////////////////////////////////////////////////////////////	
				const Storm::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoodArrays[currentPIndex];
				for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
				{
					if (neighbor._isFluidParticle)
					{
						if (neighbor._containingParticleSystem != lastNeighborFluidSystem)
						{
							lastNeighborFluidSystem = static_cast<const Storm::FluidParticleSystem*>(neighbor._containingParticleSystem);
							neighborDataArray = &_data.find(lastNeighborFluidSystem->getId())->second;
						}

						const Storm::DFSPHSolverData &neighborData = (*neighborDataArray)[neighbor._particleIndex];

						const float b_j = neighborData._predictedDensity;
						const float kj = b_j * neighborData._kCoeff;

						const float kSum = ki + lastNeighborFluidSystem->getRestDensity() / density0 * kj;
						if (std::fabs(kSum) > k_epsilon)
						{
							///// WEIRD FIXME ??? //////// => minus doesn't cancel each other unlike with rigid bodies.
#if false // Original

							const Storm::Vector3 grad_p_j = -lastNeighborFluidSystem->getParticleVolume() * neighbor._gradWij;
							v_i -= iterationParameter._deltaTime * kSum * grad_p_j;			// ki, kj already contain inverse density
#else // Optimized but not fixed
							v_i -= (iterationParameter._deltaTime * kSum * -lastNeighborFluidSystem->getParticleVolume()) * neighbor._gradWij;
#endif
						}
					}
					else if (std::fabs(ki) > k_epsilon)
					{
						Storm::RigidBodyParticleSystem* neighborPSystemAsBoundary = static_cast<Storm::RigidBodyParticleSystem*>(neighbor._containingParticleSystem);

						///// WEIRD FIXME ??? //////// => minus cancels each other unlike with fluids.
#if false // Original

						const Storm::Vector3 grad_p_j = -neighborPSystemAsBoundary->getVolumes()[neighbor._particleIndex] * neighbor._gradWij;
						const Storm::Vector3 velChange = -iterationParameter._deltaTime * 1.f * ki * grad_p_j;				// kj already contains inverse density
#else // Optimized but not fixed
						const Storm::Vector3 velChange = (iterationParameter._deltaTime * 1.f * ki * neighborPSystemAsBoundary->getVolumes()[neighbor._particleIndex]) * neighbor._gradWij;	// kj already contains inverse density
#endif
						v_i += velChange;

						if (!neighborPSystemAsBoundary->isStatic())
						{
							Storm::Vector3 addedPressureForce = (-currentPMass * invertDeltaTime) * velChange;

							Storm::Vector3 &tmpPressureForce = neighborPSystemAsBoundary->getTemporaryPressureForces()[neighbor._particleIndex];

							std::lock_guard<std::mutex> lock{ neighbor._containingParticleSystem->_mutex };
							tmpPressureForce += addedPressureForce;
						}
					}
				}
			});

			//////////////////////////////////////////////////////////////////////////
			// Update rho_adv and density error
			//////////////////////////////////////////////////////////////////////////
			Storm::runParallel(dataFieldPair.second, [&](Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex)
			{
				this->computeDensityChange(iterationParameter, fluidPSystem, &dataFieldPair.second, currentPData, currentPIndex);
				avgDensityErrAtom += density0 * currentPData._predictedDensity;
			});

			avgDensityErrAtom = avgDensityErrAtom / _totalParticleCountFl;

			// Maximal allowed density fluctuation
			// use maximal density error divided by time step size
			const float eta = invertDeltaTime * maxError * 0.01f * density0;  // maxError is given in percent
			chk = chk && (avgDensityErrAtom <= eta);

			outAverageError += avgDensityErrAtom;
		}

		outAverageError /= _totalParticleCountFl;

		++outIteration;
	} while ((!chk || (outIteration < minIter)) && (outIteration < maxIter));


	//////////////////////////////////////////////////////////////////////////
	// Multiply by h, the time step size has to be removed 
	// to make the stiffness value independent 
	// of the time step size
	//////////////////////////////////////////////////////////////////////////
	for (auto &dataFieldPair : _data)
	{
		// Since data field was made from fluid particles, no need to check.
		Storm::runParallel(dataFieldPair.second, [&iterationParameter](Storm::DFSPHSolverData &currentPData)
		{
			currentPData._kCoeff *= iterationParameter._deltaTime;
		});
	}
}

void Storm::DFSPHSolver::pressureSolve(const Storm::IterationParameter &iterationParameter, unsigned int &outIteration, float &outAverageError)
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

	const Storm::GeneralSimulationData &generalSimulConfig = configMgr.getGeneralSimulationData();

	Storm::SimulatorManager &simulMgr = Storm::SimulatorManager::instance();

	Storm::ParticleSystemContainer &particleSystems = *iterationParameter._particleSystems;

	const unsigned int minIter = generalSimulConfig._minPredictIteration;
	const unsigned int maxIter = generalSimulConfig._maxPredictIteration;
	const float maxError = generalSimulConfig._maxDensityError;

	const float deltaTimeSquared = iterationParameter._deltaTime * iterationParameter._deltaTime;
	const float invDeltaTime = 1.f / iterationParameter._deltaTime;
	const float invDeltaTimeSquared = 1.f / deltaTimeSquared;


	//////////////////////////////////////////////////////////////////////////
	// Compute rho_adv
	//////////////////////////////////////////////////////////////////////////
	for (auto &dataFieldPair : _data)
	{
		// Since data field was made from fluid particles, no need to check.
		Storm::FluidParticleSystem &fluidPSystem = static_cast<Storm::FluidParticleSystem &>(*particleSystems.find(dataFieldPair.first)->second);
		Storm::runParallel(dataFieldPair.second, [this, &iterationParameter, &fluidPSystem, &dataFieldPair, invDeltaTimeSquared](Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex)
		{
			this->computeDensityAdv(iterationParameter, fluidPSystem, &dataFieldPair.second, currentPData, currentPIndex);
			currentPData._kCoeff *= invDeltaTimeSquared;
		});
	}

	outIteration = 0;

	//////////////////////////////////////////////////////////////////////////
	// Start solver
	//////////////////////////////////////////////////////////////////////////

	bool chk;

	constexpr const float k_epsilon = 0.00001f;

	do
	{
		chk = true;

		outAverageError = 0.f;

		for (auto &dataFieldPair : _data)
		{
			// Since data field was made from fluid particles, no need to check.
			Storm::FluidParticleSystem &fluidPSystem = static_cast<Storm::FluidParticleSystem &>(*particleSystems.find(dataFieldPair.first)->second);

			const float density0 = fluidPSystem.getRestDensity();

			float densityError = 0.f;

			const std::vector<float> &masses = fluidPSystem.getMasses();
			const std::vector<Storm::ParticleNeighborhoodArray> &neighborhoodArrays = fluidPSystem.getNeighborhoodArrays();

			//////////////////////////////////////////////////////////////////////////
			// Compute pressure forces
			//////////////////////////////////////////////////////////////////////////
			Storm::runParallel(dataFieldPair.second, [&](Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex)
			{
				const float currentPMass = masses[currentPIndex];

				//////////////////////////////////////////////////////////////////////////
				// Evaluate rhs
				//////////////////////////////////////////////////////////////////////////
				const float b_i = currentPData._predictedDensity - 1.f;
				const float ki = b_i * currentPData._kCoeff;

				Storm::Vector3 &v_i = currentPData._predictedVelocity;

				const Storm::DFSPHSolver::DFSPHSolverDataArray* neighborDataArray = &dataFieldPair.second;
				const Storm::FluidParticleSystem* lastNeighborFluidSystem = &fluidPSystem;

				const Storm::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoodArrays[currentPIndex];
				for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
				{
					if (neighbor._isFluidParticle)
					{
						if (neighbor._containingParticleSystem != lastNeighborFluidSystem)
						{
							lastNeighborFluidSystem = static_cast<const Storm::FluidParticleSystem*>(neighbor._containingParticleSystem);
							neighborDataArray = &_data.find(lastNeighborFluidSystem->getId())->second;
						}

						const Storm::DFSPHSolverData &neighborData = (*neighborDataArray)[neighbor._particleIndex];

						const float b_j = neighborData._predictedDensity - 1.f;
						const float kj = b_j * neighborData._kCoeff;
						const float kSum = ki + lastNeighborFluidSystem->getRestDensity() / density0 * kj;
						if (std::fabs(kSum) > k_epsilon)
						{
							///// WEIRD FIXME ??? //////// => minus doesn't cancel each other unlike with rigid bodies.
#if false // Original
							const Storm::Vector3 grad_p_j = -lastNeighborFluidSystem->getParticleVolume() * neighbor._gradWij;

							// Directly update velocities instead of storing pressure accelerations
							v_i -= iterationParameter._deltaTime * kSum * grad_p_j;			// ki, kj already contain inverse density						
#else // Optimized but not fixed

							// Directly update velocities instead of storing pressure accelerations
							v_i -= (iterationParameter._deltaTime * kSum * -lastNeighborFluidSystem->getParticleVolume()) * neighbor._gradWij;	// ki, kj already contain inverse density
#endif
						}
					}
					else if (std::fabs(ki) > k_epsilon)
					{
						Storm::RigidBodyParticleSystem* neighborPSystemAsBoundary = static_cast<Storm::RigidBodyParticleSystem*>(neighbor._containingParticleSystem);

						///// WEIRD FIXME ??? //////// => minus cancels each other unlike with fluids.

#if false // Original
						const Storm::Vector3 grad_p_j = -neighborPSystemAsBoundary->getVolumes()[neighbor._particleIndex] * neighbor._gradWij;

						// Directly update velocities instead of storing pressure accelerations
						const Storm::Vector3 velChange = -iterationParameter._deltaTime * 1.f * ki * grad_p_j;	// kj already contains inverse density
#else // Optimized but not fixed
						// Directly update velocities instead of storing pressure accelerations
						const Storm::Vector3 velChange = (iterationParameter._deltaTime * 1.f * ki * neighborPSystemAsBoundary->getVolumes()[neighbor._particleIndex]) * neighbor._gradWij;				// kj already contains inverse density
#endif
						v_i += velChange;

						if (!neighborPSystemAsBoundary->isStatic())
						{
							Storm::Vector3 addedPressureForce = (-currentPMass * invDeltaTime) * velChange;

							Storm::Vector3 &tmpPressureForce = neighborPSystemAsBoundary->getTemporaryPressureForces()[neighbor._particleIndex];

							std::lock_guard<std::mutex> lock{ neighbor._containingParticleSystem->_mutex };
							tmpPressureForce += addedPressureForce;
						}
					}
				}
			});

			//////////////////////////////////////////////////////////////////////////
			// Update rho_adv and density error
			//////////////////////////////////////////////////////////////////////////
			Storm::runParallel(dataFieldPair.second, [&](Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex)
			{
				this->computeDensityAdv(iterationParameter, fluidPSystem, &dataFieldPair.second, currentPData, currentPIndex);
				densityError += density0 * currentPData._predictedDensity - density0;
			});

			outAverageError = densityError / _totalParticleCountFl;

			// Maximal allowed density fluctuation
			const float eta = maxError * 0.01f * density0;  // maxError is given in percent
			chk = chk && (outAverageError <= eta);
		}

		++outIteration;
	} while ((!chk || (outIteration < minIter)) && (outIteration < maxIter));
}

void Storm::DFSPHSolver::computeDFSPHFactor(const Storm::IterationParameter &iterationParameter, Storm::FluidParticleSystem &fluidPSystem, Storm::DFSPHSolver::DFSPHSolverDataArray &pSystemData)
{
	//////////////////////////////////////////////////////////////////////////
	// Init parameters
	//////////////////////////////////////////////////////////////////////////

	const std::vector<Storm::ParticleNeighborhoodArray> &neighborhoodArrays = fluidPSystem.getNeighborhoodArrays();

	constexpr const float k_epsilon = 0.00001f;

	//////////////////////////////////////////////////////////////////////////
	// Compute pressure stiffness denominator
	//////////////////////////////////////////////////////////////////////////
	Storm::runParallel(pSystemData, [&iterationParameter, &neighborhoodArrays](Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex)
	{
		const Storm::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoodArrays[currentPIndex];

		//////////////////////////////////////////////////////////////////////////
		// Compute gradient dp_i/dx_j * (1/k)  and dp_j/dx_j * (1/k)
		//////////////////////////////////////////////////////////////////////////
		float sum_grad_p_k = 0.f;
		Storm::Vector3 grad_p_i = Storm::Vector3::Zero();

		for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
		{
			if (neighbor._isFluidParticle)
			{
				const Storm::FluidParticleSystem* neighborPSystemAsFluid = static_cast<const Storm::FluidParticleSystem*>(neighbor._containingParticleSystem);
				const Storm::Vector3 grad_p_j = -neighborPSystemAsFluid->getParticleVolume() * neighbor._gradWij;
				sum_grad_p_k += grad_p_j.squaredNorm();
				grad_p_i -= grad_p_j;
			}
			else
			{
				const Storm::RigidBodyParticleSystem* neighborPSystemAsBoundary = static_cast<const Storm::RigidBodyParticleSystem*>(neighbor._containingParticleSystem);
				const Storm::Vector3 grad_p_j = -neighborPSystemAsBoundary->getVolumes()[neighbor._particleIndex] * neighbor._gradWij;
				grad_p_i -= grad_p_j;
			}
		}

		sum_grad_p_k += grad_p_i.squaredNorm();

		//////////////////////////////////////////////////////////////////////////
		// Compute pressure stiffness denominator
		//////////////////////////////////////////////////////////////////////////
		if (sum_grad_p_k > k_epsilon)
		{
			currentPData._kCoeff = -1.f / sum_grad_p_k;
		}
		else
		{
			currentPData._kCoeff = 0.f;
		}
	});
}

void Storm::DFSPHSolver::computeDensityAdv(const Storm::IterationParameter &iterationParameter, Storm::FluidParticleSystem &fluidPSystem, const Storm::DFSPHSolver::DFSPHSolverDataArray* currentSystemData, Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex)
{
	const float density = fluidPSystem.getDensities()[currentPIndex];
	float &densityAdv = currentPData._predictedDensity;

	const float density0 = fluidPSystem.getRestDensity();

	const Storm::ParticleNeighborhoodArray &currentPNeighborhood = fluidPSystem.getNeighborhoodArrays()[currentPIndex];

	const Storm::Vector3 &vi = currentPData._predictedVelocity;

	const Storm::DFSPHSolver::DFSPHSolverDataArray* neighborDataArray = currentSystemData;
	const Storm::FluidParticleSystem* lastNeighborFluidSystem = &fluidPSystem;

	float delta = 0.f;
	for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
	{
		if (neighbor._isFluidParticle)
		{
			if (neighbor._containingParticleSystem != lastNeighborFluidSystem)
			{
				lastNeighborFluidSystem = static_cast<const Storm::FluidParticleSystem*>(neighbor._containingParticleSystem);
				neighborDataArray = &_data.find(lastNeighborFluidSystem->getId())->second;
			}

			const Storm::DFSPHSolverData &neighborData = (*neighborDataArray)[neighbor._particleIndex];

			const Storm::Vector3 &vj = neighborData._predictedVelocity;
			delta += lastNeighborFluidSystem->getParticleVolume() * (vi - vj).dot(neighbor._gradWij);
		}
		else
		{
			const Storm::RigidBodyParticleSystem* neighborPSystemAsBoundary = static_cast<const Storm::RigidBodyParticleSystem*>(neighbor._containingParticleSystem);
			const Storm::Vector3 &vj = neighborPSystemAsBoundary->getVelocity()[neighbor._particleIndex];
			delta += neighborPSystemAsBoundary->getVolumes()[neighbor._particleIndex] * (vi - vj).dot(neighbor._gradWij);
		}
	}

	densityAdv = density / density0 + iterationParameter._deltaTime * delta;
	densityAdv = std::max(densityAdv, 1.f);
}

void Storm::DFSPHSolver::computeDensityChange(const Storm::IterationParameter &iterationParameter, Storm::FluidParticleSystem &fluidPSystem, const Storm::DFSPHSolver::DFSPHSolverDataArray* currentSystemData, Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex)
{
	const Storm::ParticleNeighborhoodArray &currentPNeighborhood = fluidPSystem.getNeighborhoodArrays()[currentPIndex];

	// in case of particle deficiency do not perform a divergence solve
	if (currentPNeighborhood.size() >= 20)
	{
		float &densityAdv = currentPData._predictedDensity;
		densityAdv = 0.f;

		const Storm::DFSPHSolver::DFSPHSolverDataArray* neighborDataArray = currentSystemData;
		const Storm::FluidParticleSystem* lastNeighborFluidSystem = &fluidPSystem;

		const Storm::Vector3 &vi = currentPData._predictedVelocity;

		for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
		{
			if (neighbor._isFluidParticle)
			{
				if (neighbor._containingParticleSystem != lastNeighborFluidSystem)
				{
					lastNeighborFluidSystem = static_cast<const Storm::FluidParticleSystem*>(neighbor._containingParticleSystem);
					neighborDataArray = &_data.find(lastNeighborFluidSystem->getId())->second;
				}

				const Storm::DFSPHSolverData &neighborData = (*neighborDataArray)[neighbor._particleIndex];

				const Storm::Vector3 &vj = neighborData._predictedVelocity;
				densityAdv += lastNeighborFluidSystem->getParticleVolume() * (vi - vj).dot(neighbor._gradWij);
			}
			else
			{
				const Storm::RigidBodyParticleSystem* neighborPSystemAsBoundary = static_cast<const Storm::RigidBodyParticleSystem*>(neighbor._containingParticleSystem);
				const Storm::Vector3 &vj = neighborPSystemAsBoundary->getVelocity()[neighbor._particleIndex];
				densityAdv += neighborPSystemAsBoundary->getVolumes()[neighbor._particleIndex] * (vi - vj).dot(neighbor._gradWij);
			}
		}

		// only correct positive divergence
		densityAdv = std::max(densityAdv, 0.f);
	}
}
