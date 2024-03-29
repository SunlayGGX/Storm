//
// Note :
// This implementation was taken from the implementation J.Bender & Al. did inside SplishSplash
// and re adapted to suit how my custom engine works.
//
//

// ReSharper disable CppClangTidyCppcoreguidelinesProTypeStaticCastDowncast
#include "DFSPHSolver.h"

#include "ParticleSystem.h"
#include "FluidParticleSystem.h"
#include "RigidBodyParticleSystem.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"
#include "SimulatorManager.h"

#include "SceneSimulationConfig.h"
#include "SceneFluidConfig.h"
#include "SceneFluidCustomDFSPHConfig.h"

#include "Kernel.h"
#include "ViscosityMethod.h"

#include "DFSPHSolverData.h"

#include "IterationParameter.h"

#define STORM_HIJACKED_TYPE Storm::DFSPHSolverData
#	include "VectHijack.h"
#undef STORM_HIJACKED_TYPE

#include "RunnerHelper.h"
#include "SPHSolverUtils.h"


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

	template<Storm::ViscosityMethod viscosityMethodOnFluid, Storm::ViscosityMethod viscosityMethodOnRigidBody>
	Storm::Vector3 computeViscosity(const Storm::IterationParameter &iterationParameter, const Storm::SceneFluidConfig &fluidConfig, const Storm::FluidParticleSystem &fluidParticleSystem, const float currentPMass, const Storm::Vector3 &vi, const Storm::ParticleNeighborhoodArray &currentPNeighborhood, const float currentPDensity, const float viscoPrecoeff)
	{
		Storm::Vector3 totalViscosityForceOnParticle = Storm::Vector3::Zero();

		const float density0 = fluidParticleSystem.getRestDensity();

		// Even though reflected particle are real particle inside the domain, they are in fact dummies to represent the notion of outside of domain particle
		// And we consider out of domain particle to be still air (or at least, we don't care about their velocity).
		// In other words, we consider reflected particle, aka out of domain particle, to have no velocity.
		const Storm::Vector3 k_velocityInCasePReflected = Storm::Vector3::Zero();

		for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
		{
			const Storm::Vector3 &vj = neighbor._notReflected ? neighbor._containingParticleSystem->getVelocity()[neighbor._particleIndex] : k_velocityInCasePReflected;
			const Storm::Vector3 vij = vi - vj;

			const float vijDotXij = vij.dot(neighbor._xij);
			const float viscoGlobalCoeff = currentPMass * 10.f * vijDotXij / (neighbor._xijSquaredNorm + viscoPrecoeff);

			Storm::Vector3 viscosityComponent;

			if (neighbor._isFluidParticle)
			{
				const Storm::FluidParticleSystem* neighborPSystemAsFluid = static_cast<Storm::FluidParticleSystem*>(neighbor._containingParticleSystem);  // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
				const float neighborDensity0 = neighborPSystemAsFluid->getRestDensity();
				const float neighborMass = neighborPSystemAsFluid->getMasses()[neighbor._particleIndex];
				const float neighborRawDensity = neighborPSystemAsFluid->getDensities()[neighbor._particleIndex];
				const float neighborDensity = neighborRawDensity * density0 / neighborDensity0;

				if constexpr (viscosityMethodOnFluid == Storm::ViscosityMethod::Standard)
				{
					viscosityComponent = (viscoGlobalCoeff * fluidConfig._cinematicViscosity * neighborMass / neighborRawDensity) * neighbor._gradWij;
				}
				else if constexpr (viscosityMethodOnFluid == Storm::ViscosityMethod::XSPH)
				{
					viscosityComponent = (-(currentPMass * neighborMass * fluidConfig._cinematicViscosity / (iterationParameter._deltaTime * neighborDensity)) * neighbor._Wij) * vij;
				}
				else
				{
					Storm::throwException<Storm::Exception>("Non implemented viscosity method to use on fluid!");
				}
			}
			else
			{
				const Storm::RigidBodyParticleSystem* neighborPSystemAsBoundary = static_cast<Storm::RigidBodyParticleSystem*>(neighbor._containingParticleSystem);

				const float rbViscosity = neighborPSystemAsBoundary->getViscosity();

				// Viscosity
				if (rbViscosity > 0.f)
				{
					const float neighborVolume = neighborPSystemAsBoundary->getVolumes()[neighbor._particleIndex];
					if constexpr (viscosityMethodOnRigidBody == Storm::ViscosityMethod::Standard)
					{
						viscosityComponent = (viscoGlobalCoeff * rbViscosity * neighborVolume * density0 / currentPDensity) * neighbor._gradWij;
					}
					else if constexpr (viscosityMethodOnRigidBody == Storm::ViscosityMethod::XSPH)
					{
						viscosityComponent = (-(currentPMass * rbViscosity * neighborVolume * density0 / (iterationParameter._deltaTime * currentPDensity)) * neighbor._Wij) * vij;
					}
					else
					{
						Storm::throwException<Storm::Exception>("Non implemented viscosity method to use on rigid body!");
					}
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

		return totalViscosityForceOnParticle;
	}

	Storm::Vector3 computeBernoulliPrinciple(const Storm::FluidParticleSystem &fluidParticleSystem, const std::size_t currentPIndex, const Storm::ParticleNeighborhoodArray &currentPNeighborhood)
	{
		// Explanation :
		// SPH computes static pressure in the fluids. But total pressure is given by the sum between static and dynamic pressure.
		// Plus, SPH clamps static pressure at 0 when density is below rest density, therefore, no static pressure forces allow to correct dilatation (just the compression of the flow).
		// Dynamic pressure is the one that pulls the ball in the blower flow thus the need to compute it.
		// From Bernoulli's equations given for Incompressible flow (Bernoulli's principle), dynamic pressure is given by : q = 1/2 * rho * v�. This has a dimension of a pressure (N.m^-2). Therefore Acceleration from dynamic pressure is given by aq = 1/2 * rho / rho0 * v�


		Storm::Vector3 dynamicPressureForce = Storm::Vector3::Zero();

		const float density0 = fluidParticleSystem.getRestDensity();
		const float currentPMass = fluidParticleSystem.getMasses()[currentPIndex];
		const float currentPDensity = fluidParticleSystem.getDensities()[currentPIndex];
		const float qi = fluidParticleSystem.getPressures()[currentPIndex];

		const float currentPFluidPressureCoeff = qi / (currentPDensity * currentPDensity);

		const float restMassDensity = currentPMass * density0;
		const float currentPRestMassDensityBoundaryPressureCoeff = restMassDensity * (currentPFluidPressureCoeff + (qi / (density0 * density0)));

		for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
		{
#if true
			if (neighbor._isFluidParticle)
			{
				const Storm::FluidParticleSystem* neighborPSystemAsFluid = static_cast<Storm::FluidParticleSystem*>(neighbor._containingParticleSystem);
				const float neighborDensity0 = neighborPSystemAsFluid->getRestDensity();
				const float neighborRawDensity = neighborPSystemAsFluid->getDensities()[neighbor._particleIndex];
				const float neighborDensity = neighborRawDensity * (density0 / neighborDensity0);
				const float neighborVolume = neighborPSystemAsFluid->getParticleVolume();

				const float neighborPressureCoeff = neighborPSystemAsFluid->getPressures()[neighbor._particleIndex] / (neighborDensity * neighborDensity);
				dynamicPressureForce += (restMassDensity * neighborVolume * (currentPFluidPressureCoeff + neighborPressureCoeff)) * neighbor._gradWij;
			}
			else
			{
				const Storm::RigidBodyParticleSystem* neighborPSystemAsBoundary = static_cast<Storm::RigidBodyParticleSystem*>(neighbor._containingParticleSystem);

				const float neighborVolume = neighborPSystemAsBoundary->getVolumes()[neighbor._particleIndex];

				// Pressure
				const Storm::Vector3 currentDynamicPressureQComponent = (currentPRestMassDensityBoundaryPressureCoeff * neighborVolume) * neighbor._gradWij;
				dynamicPressureForce += currentDynamicPressureQComponent;

				// Mirror the force on the boundary solid following the 3rd newton law
				if (!neighborPSystemAsBoundary->isStatic())
				{
					Storm::Vector3 &boundaryNeighborTmpDynamicQForce = neighbor._containingParticleSystem->getTemporaryBernoulliDynamicPressureForces()[neighbor._particleIndex];

					std::lock_guard<std::mutex> lock{ neighbor._containingParticleSystem->_mutex };
					boundaryNeighborTmpDynamicQForce -= currentDynamicPressureQComponent;
				}
			}
#else
			STORM_TODO("Consider reflected particle (aka out of domain particle) velocity instead.");

			if (neighbor._isFluidParticle)
			{
				const Storm::FluidParticleSystem*const neighborPSystemAsFluid = static_cast<Storm::FluidParticleSystem*>(neighbor._containingParticleSystem);
				const float neighborMass = neighborPSystemAsFluid->getMasses()[neighbor._particleIndex];
				const float neighborRawDensity = neighborPSystemAsFluid->getDensities()[neighbor._particleIndex];
				const float particleVolume = neighborPSystemAsFluid->getParticleVolume();

				const Storm::Vector3 currentDynamicPressureQComponent = (particleVolume * neighborMass / neighborRawDensity * (qi - neighborPSystemAsFluid->getPressures()[neighbor._particleIndex])) * neighbor._gradWij;

				dynamicPressureForce -= currentDynamicPressureQComponent;
			}
			else
			{
				const Storm::RigidBodyParticleSystem*const neighborPSystemAsBoundary = static_cast<Storm::RigidBodyParticleSystem*>(neighbor._containingParticleSystem);
				const float neighborVolume = neighborPSystemAsBoundary->getVolumes()[neighbor._particleIndex];

				// Even though reflected particle are real particle inside the domain, they are in fact dummies to represent the notion of outside of domain particle
				// And we consider out of domain particle to be still fluid at density0.
				const float qj = neighbor._notReflected ? (density0 * neighborPSystemAsBoundary->getVelocity()[neighbor._particleIndex].squaredNorm() / 2.f) : density0;

				const Storm::Vector3 currentDynamicPressureQComponent = (neighborVolume * neighborVolume * (qj - qi)) * neighbor._gradWij;

				dynamicPressureForce -= currentDynamicPressureQComponent;

				// Mirror the force on the boundary solid following the 3rd newton law
				if (!neighborPSystemAsBoundary->isStatic())
				{
					Storm::Vector3 &boundaryNeighborTmpDynamicQForce = neighbor._containingParticleSystem->getTemporaryBernoulliDynamicPressureForces()[neighbor._particleIndex];

					std::lock_guard<std::mutex> lock{ neighbor._containingParticleSystem->_mutex };
					boundaryNeighborTmpDynamicQForce += currentDynamicPressureQComponent;
				}
			}

#endif
		}

		return dynamicPressureForce;
	}

	Storm::Vector3 produceNoStickConditionForces(const Storm::IterationParameter &iterationParameter, const Storm::FluidParticleSystem &/*fluidParticleSystem*/, const Storm::Vector3 &vi, const float currentPMass, const Storm::ParticleNeighborhoodArray &currentPNeighborhood)
	{
		Storm::Vector3 result = Storm::Vector3::Zero();

		const float velocityToForceCoeff = currentPMass / iterationParameter._deltaTime;
		for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
		{
			if (!neighbor._isFluidParticle)
			{
				Storm::RigidBodyParticleSystem &pSystemAsRb = static_cast<Storm::RigidBodyParticleSystem &>(*neighbor._containingParticleSystem);

				const float noStickCoeff = pSystemAsRb.getNoStickCoefficient();
				if (noStickCoeff > 0.f)
				{
					const Storm::Vector3 velDiff = neighbor._notReflected ? (vi - pSystemAsRb.getVelocity()[neighbor._particleIndex]) : vi;
					const Storm::Vector3 &rbPNormal = pSystemAsRb.getNormals()[neighbor._particleIndex];

					// Make it the component that removes the normal component of the velocity.
					// Note : we suppose the rigid body normal at neighbor particle (rbPNormal) is normalized.
					const Storm::Vector3 addedForce = (velDiff.dot(rbPNormal) * velocityToForceCoeff * noStickCoeff * neighbor._Wij * iterationParameter._kernelLengthSquared) * rbPNormal;
					result -= addedForce;

					// Mirror the force on the boundary solid following the 3rd newton law
					if (!pSystemAsRb.isStatic())
					{
						Storm::Vector3 &boundaryNeighborTmpNoStickForce = neighbor._containingParticleSystem->getTemporaryNoStickForces()[neighbor._particleIndex];

						std::lock_guard<std::mutex> lock{ neighbor._containingParticleSystem->_mutex };
						boundaryNeighborTmpNoStickForce += addedForce;
					}
				}
			}
		}

		return result;
	}

	Storm::Vector3 produceCoandaForces(const Storm::IterationParameter &iterationParameter, const Storm::FluidParticleSystem &fluidParticleSystem, const Storm::Vector3 &/*vi*/, const float currentPMass, const Storm::ParticleNeighborhoodArray &currentPNeighborhood)
	{
		Storm::Vector3 result = Storm::Vector3::Zero();

		const float restDensity = fluidParticleSystem.getRestDensity();
		const float deltaTimeSquaredCoeff = 2.f * iterationParameter._deltaTime * iterationParameter._deltaTime;
		const float toForceICoeff = currentPMass / deltaTimeSquaredCoeff;
		for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
		{
			if (!neighbor._isFluidParticle)
			{
				Storm::RigidBodyParticleSystem &pSystemAsRb = static_cast<Storm::RigidBodyParticleSystem &>(*neighbor._containingParticleSystem);

				const float coandaCoeff = pSystemAsRb.getCoandaCoefficient();
				if (coandaCoeff > 0.f)
				{
					const float toForceJCoeff = pSystemAsRb.getVolumes()[neighbor._particleIndex] * restDensity / deltaTimeSquaredCoeff;

					// Make it the component that removes the normal component of the velocity.
					// Note : we suppose the rigid body normal at neighbor particle (rbPNormal) is normalized.
					const Storm::Vector3 addedForce = ((toForceICoeff + toForceJCoeff) * coandaCoeff) * neighbor._xij;
					result -= addedForce;

					// Mirror the force on the boundary solid following the 3rd newton law
					if (!pSystemAsRb.isStatic())
					{
						Storm::Vector3 &boundaryNeighborTmpCoandaForce = neighbor._containingParticleSystem->getTemporaryCoandaForces()[neighbor._particleIndex];

						std::lock_guard<std::mutex> lock{ neighbor._containingParticleSystem->_mutex };
						boundaryNeighborTmpCoandaForce += addedForce;
					}
				}
			}
		}

		return result;
	}
}


Storm::DFSPHSolver::DFSPHSolver(const float /*k_kernelLength*/, const Storm::ParticleSystemContainer &particleSystemsMap) :
	Storm::PredictiveSolverHandler{ g_solverIterationNames, g_solverErrorsNames }
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
				currentPData._densityAdv = 0.f;
			});

			totalParticleCount += currentPSystemPCount._newSize;
		}
	}

	_totalParticleCountFl = static_cast<float>(totalParticleCount);

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

	const Storm::SceneFluidConfig &sceneFluidConfig = configMgr.getSceneFluidConfig();

	const Storm::SceneFluidCustomDFSPHConfig &dfsphFluidConfig = static_cast<const Storm::SceneFluidCustomDFSPHConfig &>(*sceneFluidConfig._customSimulationSettings);
	_enableThresholdDensity = dfsphFluidConfig._enableThresholdDensity;
	_neighborThresholdDensity = dfsphFluidConfig._neighborThresholdDensity;
	_useRotationFix = dfsphFluidConfig._useFixRotation;
	_enableDensitySolve = dfsphFluidConfig._enableDensitySolve;
}

Storm::DFSPHSolver::~DFSPHSolver() = default;

void Storm::DFSPHSolver::execute(const Storm::IterationParameter &iterationParameter)
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

	const Storm::SceneSimulationConfig &sceneSimulationConfig = configMgr.getSceneSimulationConfig();
	const Storm::SceneFluidConfig &fluidConfig = configMgr.getSceneFluidConfig();
	const Storm::SceneFluidCustomDFSPHConfig &dfsphFluidConfig = static_cast<const Storm::SceneFluidCustomDFSPHConfig &>(*fluidConfig._customSimulationSettings);

	Storm::SimulatorManager &simulMgr = Storm::SimulatorManager::instance();

	Storm::ParticleSystemContainer &particleSystems = *iterationParameter._particleSystems;

	// 1st : Initialize iteration
	simulMgr.advanceBlowersTime(iterationParameter._deltaTime);
	simulMgr.refreshParticleNeighborhood();
	simulMgr.subIterationStart();

	// 2nd : refresh particle neighborhood
	//simulMgr.refreshParticleNeighborhood();

	if (!this->shouldContinue()) STORM_UNLIKELY
	{
		return;
	}

	// 3rd : Compute the base density
	this->initializeStepDensities(iterationParameter, sceneSimulationConfig, dfsphFluidConfig);
	if (!this->shouldContinue()) STORM_UNLIKELY
	{
		return;
	}

	// 4th : Compute k_dfsph coeff
	const double kPressurePredictFinalCoeff = static_cast<double>(-dfsphFluidConfig._kPressurePredictedCoeff);
	for (auto &dataFieldPair : _data)
	{
		this->computeDFSPHFactor(
			iterationParameter,
			static_cast<Storm::FluidParticleSystem &>(*particleSystems.find(dataFieldPair.first)->second), // Since data field was made from fluid particles, no need to check.
			dataFieldPair.second,
			kPressurePredictFinalCoeff
		);
	}

	if (!this->shouldContinue()) STORM_UNLIKELY
	{
		return;
	}

	if (_useRotationFix)
	{
		for (auto &dataFieldPair : _data)
		{
			// Since data field was made from fluid particles, no need to check.
			const Storm::FluidParticleSystem &fluidPSystem = static_cast<const Storm::FluidParticleSystem &>(*particleSystems.find(dataFieldPair.first)->second);
			const std::vector<Storm::Vector3> &velocities = fluidPSystem.getVelocity();
			Storm::runParallel(dataFieldPair.second, [this, &velocities](Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex)
			{
				currentPData._predictedVelocity = velocities[currentPIndex];
			});
		}

		if (!this->shouldContinue()) STORM_UNLIKELY
		{
			return;
		}
	}

	// 5th : Divergence solve
	this->fullVelocityDivergenceSolve_Internal(iterationParameter, fluidConfig, dfsphFluidConfig);
	if (!this->shouldContinue()) STORM_UNLIKELY
	{
		return;
	}

	// 7th : Compute the non pressure forces (viscosity)
	// Compute accelerations: a(t)
	this->computeNonPressureForces_Internal(iterationParameter, sceneSimulationConfig, fluidConfig);
	if (!this->shouldContinue()) STORM_UNLIKELY
	{
		return;
	}

	if (sceneSimulationConfig._midUpdateViscosity)
	{
		// Update Rb velocities with viscosity (experimental)
		for (auto &particleSystemPair : particleSystems)
		{
			Storm::ParticleSystem &currentParticleSystem = *particleSystemPair.second;
			if (!currentParticleSystem.isFluids())
			{
				Storm::RigidBodyParticleSystem &rbParticleSystem = static_cast<Storm::RigidBodyParticleSystem &>(currentParticleSystem);

				const std::vector<Storm::Vector3> &viscoForces = rbParticleSystem.getTemporaryViscosityForces();
				std::vector<Storm::Vector3> &forces = rbParticleSystem.getForces();

				std::copy(std::execution::par, std::begin(viscoForces), std::end(viscoForces), std::begin(forces));
			}
		}

		simulMgr.flushPhysics(iterationParameter._deltaTime);
		for (auto &particleSystemPair : particleSystems)
		{
			Storm::ParticleSystem &currentParticleSystem = *particleSystemPair.second;
			if (!currentParticleSystem.isFluids())
			{
				Storm::RigidBodyParticleSystem &rbParticleSystem = static_cast<Storm::RigidBodyParticleSystem &>(currentParticleSystem);

				rbParticleSystem.updatePosition(iterationParameter._deltaTime, false);
			}
		}

		simulMgr.refreshParticleNeighborhood();
	}

	// 8th : TODO : CFL
	// ...

	// 9th : Solve pressure
	this->fullDensityInvariantSolve_Internal(iterationParameter, dfsphFluidConfig);
	if (!this->shouldContinue()) STORM_UNLIKELY
	{
		return;
	}

	///////////////////////////////////////////////////

	// 10th : compute final positions
	this->transfertEndDataToSystems(particleSystems, iterationParameter, &_data, [](void* data, const unsigned int pSystemId, Storm::FluidParticleSystem &fluidParticleSystem, const Storm::IterationParameter &iterationParameter)
	{
		auto &dataField = static_cast<decltype(_data)*>(data)->find(pSystemId)->second;

		const std::vector<float> &masses = fluidParticleSystem.getMasses();
		std::vector<Storm::Vector3> &forces = fluidParticleSystem.getForces();
		std::vector<Storm::Vector3> &velocities = fluidParticleSystem.getVelocity();
		std::vector<Storm::Vector3> &positions = fluidParticleSystem.getPositions();
		std::vector<Storm::Vector3> &tmpPressureForces = fluidParticleSystem.getTemporaryPressureForces();

		const std::vector<Storm::Vector3> &tmpPressureDensityForces = fluidParticleSystem.getTemporaryPressureDensityIntermediaryForces();
		const std::vector<Storm::Vector3> &tmpPressureVelocityForces = fluidParticleSystem.getTemporaryPressureVelocityIntermediaryForces();
		const std::vector<Storm::Vector3> &velocityPreTimestep = fluidParticleSystem.getVelocityPreTimestep();

		std::atomic<bool> dirtyTmp = false;
#pragma warning (push)
#pragma warning (disable: 4189) // It is being used, but the compiler cannot see it before it compiles for real
		constexpr const float minForceDirtyEpsilon = 0.0001f;
#pragma warning (push)

		Storm::runParallel(dataField, [&](const Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex)
		{
			// Euler integration
			Storm::Vector3 &currentPVelocity = velocities[currentPIndex];
			Storm::Vector3 &currentPPosition = positions[currentPIndex];

			tmpPressureForces[currentPIndex] = tmpPressureDensityForces[currentPIndex] + tmpPressureVelocityForces[currentPIndex];

			forces[currentPIndex] = (currentPData._predictedVelocity - velocityPreTimestep[currentPIndex]) * (masses[currentPIndex] / iterationParameter._deltaTime);

			currentPVelocity = currentPData._predictedVelocity;
			currentPPosition += currentPVelocity * iterationParameter._deltaTime;

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
	}, !sceneSimulationConfig._midUpdateViscosity);

	// 11th : flush physics state (rigid bodies)
	simulMgr.flushPhysics(iterationParameter._deltaTime);
}

void Storm::DFSPHSolver::removeRawEndData(const unsigned int pSystemId, std::size_t toRemoveCount)
{
	Storm::SPHSolverUtils::removeRawEndData(pSystemId, toRemoveCount, _data);
}

void Storm::DFSPHSolver::setEnableThresholdDensity(bool enable)
{
	_enableThresholdDensity = enable;
}

void Storm::DFSPHSolver::setUseRotationFix(bool useFix)
{
	_useRotationFix = useFix;
}

void Storm::DFSPHSolver::setNeighborThresholdDensity(std::size_t neighborCount)
{
	_neighborThresholdDensity = neighborCount;
}


void Storm::DFSPHSolver::initializeStepDensities(const Storm::IterationParameter &iterationParameter, const Storm::SceneSimulationConfig &sceneSimulationConfig, const Storm::SceneFluidCustomDFSPHConfig &sceneDFSPHSimulationConfig)
{
	const float k_kernelZero = Storm::retrieveKernelZeroValue(sceneSimulationConfig._kernelMode);
	Storm::ParticleSystemContainer &particleSystems = *iterationParameter._particleSystems;

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

				/*const std::vector<Storm::Vector3> &velocities = fluidParticleSystem.getVelocity();
				const Storm::Vector3 &vi = velocities[currentPIndex];
				fluidParticleSystem.getPressures()[currentPIndex] = vi.squaredNorm() / 2.f * particleVolume * k_kernelZero;*/
			});
		}
	}

	if (sceneDFSPHSimulationConfig._useBernoulliPrinciple)
	{
		for (auto &particleSystemPair : particleSystems)
		{
			Storm::ParticleSystem &currentParticleSystem = *particleSystemPair.second;
			if (currentParticleSystem.isFluids())
			{
				Storm::FluidParticleSystem &fluidPSystem = static_cast<Storm::FluidParticleSystem &>(currentParticleSystem);

				const std::vector<Storm::ParticleNeighborhoodArray> &neighborhoodArrays = fluidPSystem.getNeighborhoodArrays();
				const std::vector<Storm::Vector3> &velocities = fluidPSystem.getVelocity();
				const std::vector<float> &densities = fluidPSystem.getDensities();

				// Even though reflected particle are real particle inside the domain, they are in fact dummies to represent the notion of outside of domain particle
				// And we consider out of domain particle to be still air (or at least, we don't care about their velocity).
				// In other words, we consider reflected particle, aka out of domain particle, to have no velocity.
				Storm::runParallel(fluidPSystem.getPressures(), [this, &neighborhoodArrays, &velocities, &densities, density0 = fluidPSystem.getRestDensity(), k_velocityInCasePReflected = Storm::Vector3::Zero()](float &currentPQ, const std::size_t currentPIndex)
				{
					const Storm::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoodArrays[currentPIndex];
#if false
					const Storm::Vector3 &vi = velocities[currentPIndex];
					currentPQ = vi.squaredNorm() / 2.f * densities[currentPIndex];
#else
					currentPQ = 0.f;
					for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
					{
						const Storm::Vector3 &vj = neighbor._notReflected ? neighbor._containingParticleSystem->getVelocity()[neighbor._particleIndex] : k_velocityInCasePReflected;
						if (neighbor._isFluidParticle)
						{
							const Storm::FluidParticleSystem* neighborPSystemAsFluid = static_cast<Storm::FluidParticleSystem*>(neighbor._containingParticleSystem);
							const float neighborMass = neighborPSystemAsFluid->getMasses()[neighbor._particleIndex];

							const float addDynamicQ = vj.squaredNorm() / 2.f * neighborMass * neighbor._Wij;
							currentPQ += addDynamicQ;
						}
						else
						{
							const Storm::RigidBodyParticleSystem* neighborPSystemAsBoundary = static_cast<Storm::RigidBodyParticleSystem*>(neighbor._containingParticleSystem);
							const float neighborVolume = neighborPSystemAsBoundary->getVolumes()[neighbor._particleIndex];
							const float addDynamicQ = vj.squaredNorm() / 2.f * neighborVolume * density0 * neighbor._Wij;

							currentPQ += addDynamicQ;
						}
					}

					if (currentPQ < 0.f)
					{
						currentPQ = 0.f;
					}
#endif
				});
			}
		}
	}
}

void Storm::DFSPHSolver::fullVelocityDivergenceSolve_Internal(const Storm::IterationParameter &iterationParameter, const Storm::SceneFluidConfig &/*scenefluidConfig*/, const Storm::SceneFluidCustomDFSPHConfig &sceneDFSPHSimulationConfig)
{
	unsigned int iterationV;
	float averageErrorV;
	if (_enableDensitySolve)
	{
		this->divergenceSolve(iterationParameter, sceneDFSPHSimulationConfig, iterationV, averageErrorV);

		if (!this->shouldContinue()) STORM_UNLIKELY
		{
			return;
		}
	}
	else
	{
		iterationV = 0;
		averageErrorV = 0.f;
	}

	this->updateCurrentPredictionIter(iterationV, sceneDFSPHSimulationConfig._maxPredictIteration, averageErrorV, sceneDFSPHSimulationConfig._maxDensityError, 0);
#if false
	Storm::ParticleSystemContainer &particleSystems = *iterationParameter._particleSystems;
	for (auto &particleSystemPair : particleSystems)
	{
		Storm::ParticleSystem &currentParticleSystem = *particleSystemPair.second;
		if (currentParticleSystem.isFluids())
		{
			Storm::FluidParticleSystem &fluidParticleSystem = static_cast<Storm::FluidParticleSystem &>(currentParticleSystem);
			std::vector<float> &densities = fluidParticleSystem.getDensities();
			const float particleVolume = fluidParticleSystem.getParticleVolume();

			Storm::runParallel(fluidParticleSystem.getMasses(), [&](float &currentPMass, const std::size_t currentPIndex)
			{
				float &currentPDensity = densities[currentPIndex];
				currentPMass = currentPDensity * particleVolume * scenefluidConfig._reducedMassCoefficient;
			});
		}
	}
#endif
}


void Storm::DFSPHSolver::fullDensityInvariantSolve_Internal(const Storm::IterationParameter &iterationParameter, const Storm::SceneFluidCustomDFSPHConfig &sceneDFSPHSimulationConfig)
{
	unsigned int iteration;
	float averageError;
	this->pressureSolve(iterationParameter, sceneDFSPHSimulationConfig, iteration, averageError);

	this->updateCurrentPredictionIter(iteration, sceneDFSPHSimulationConfig._maxPredictIteration, averageError, sceneDFSPHSimulationConfig._maxPressureError, 1);
}


void Storm::DFSPHSolver::computeNonPressureForces_Internal(const Storm::IterationParameter &iterationParameter, const Storm::SceneSimulationConfig &sceneSimulationConfig, const Storm::SceneFluidConfig &fluidConfig)
{
	Storm::ParticleSystemContainer &particleSystems = *iterationParameter._particleSystems;
	const Storm::SceneFluidCustomDFSPHConfig &dfsphFluidConfig = static_cast<const Storm::SceneFluidCustomDFSPHConfig &>(*fluidConfig._customSimulationSettings);

	for (auto &dataFieldPair : _data)
	{
		// Since data field was made from fluid particles, no need to check.
		Storm::FluidParticleSystem &fluidPSystem = static_cast<Storm::FluidParticleSystem &>(*particleSystems.find(dataFieldPair.first)->second);
		std::vector<Storm::Vector3> &velocities = fluidPSystem.getVelocity();

		Storm::runParallel(dataFieldPair.second, [this, &velocities](const Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex)
		{
			velocities[currentPIndex] = currentPData._predictedVelocity;
		});
	}

	for (auto &particleSystemPair : particleSystems)
	{
		Storm::ParticleSystem &currentParticleSystem = *particleSystemPair.second;
		if (currentParticleSystem.isFluids())
		{
			Storm::FluidParticleSystem &fluidParticleSystem = static_cast<Storm::FluidParticleSystem &>(currentParticleSystem);
			
			const std::vector<Storm::ParticleNeighborhoodArray> &neighborhoodArrays = currentParticleSystem.getNeighborhoodArrays();
			std::vector<float> &masses = fluidParticleSystem.getMasses();
			const std::vector<float> &densities = fluidParticleSystem.getDensities();
			std::vector<Storm::Vector3> &temporaryPViscoForces = fluidParticleSystem.getTemporaryViscosityForces();

			const std::vector<Storm::Vector3> &velocities = fluidParticleSystem.getVelocity();

			const float viscoPrecoeff = 0.01f * iterationParameter._kernelLengthSquared;

			Storm::DFSPHSolver::DFSPHSolverDataArray &dataField = _data.find(particleSystemPair.first)->second;

			Storm::runParallel(fluidParticleSystem.getForces(), [&](Storm::Vector3 &currentPForce, const std::size_t currentPIndex)
			{
				const float currentPMass = masses[currentPIndex];
				const Storm::Vector3 &vi = velocities[currentPIndex];

				Storm::Vector3 &currentPTmpViscoForce = temporaryPViscoForces[currentPIndex];

				const float currentPDensity = densities[currentPIndex];
				const Storm::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoodArrays[currentPIndex];

#define STORM_COMPUTE_VISCOSITY(fluidMethod, rbMethod) \
	computeViscosity<fluidMethod, rbMethod>(iterationParameter, fluidConfig, fluidParticleSystem, currentPMass, vi, currentPNeighborhood, currentPDensity, viscoPrecoeff)

				switch (sceneSimulationConfig._fluidViscoMethod)
				{
				case Storm::ViscosityMethod::Standard:
					switch (sceneSimulationConfig._rbViscoMethod)
					{
					case Storm::ViscosityMethod::Standard:
						currentPTmpViscoForce = STORM_COMPUTE_VISCOSITY(Storm::ViscosityMethod::Standard, Storm::ViscosityMethod::Standard);
						break;
					case Storm::ViscosityMethod::XSPH:
						currentPTmpViscoForce = STORM_COMPUTE_VISCOSITY(Storm::ViscosityMethod::Standard, Storm::ViscosityMethod::XSPH);
						break;

					default:
						Storm::throwException<Storm::Exception>("Non implemented viscosity method to use on rigid body!");
					}
					break;

				case Storm::ViscosityMethod::XSPH:
					switch (sceneSimulationConfig._rbViscoMethod)
					{
					case Storm::ViscosityMethod::Standard:
						currentPTmpViscoForce = STORM_COMPUTE_VISCOSITY(Storm::ViscosityMethod::XSPH, Storm::ViscosityMethod::Standard);
						break;
					case Storm::ViscosityMethod::XSPH:
						currentPTmpViscoForce = STORM_COMPUTE_VISCOSITY(Storm::ViscosityMethod::XSPH, Storm::ViscosityMethod::XSPH);
						break;

					default:
						Storm::throwException<Storm::Exception>("Non implemented viscosity method to use on rigid body!");
					}

				default:
					Storm::throwException<Storm::Exception>("Non implemented viscosity method to use on fluid!");
				}

#undef STORM_COMPUTE_VISCOSITY

				currentPForce += currentPTmpViscoForce;

				if (sceneSimulationConfig._applyDragEffect)
				{
					Storm::Vector3 &currentPTmpDragForceComponent = fluidParticleSystem.getTemporaryDragForces()[currentPIndex];

					if (fluidConfig._uniformDragCoefficient > 0.f)
					{
						currentPTmpDragForceComponent = Storm::SPHSolverUtils::computeSumDragForce<true>(iterationParameter, fluidConfig._uniformDragCoefficient, fluidParticleSystem, vi, currentPNeighborhood, currentPDensity);
					}
					else
					{
						currentPTmpDragForceComponent = Storm::SPHSolverUtils::computeSumDragForce<false>(iterationParameter, fluidConfig._uniformDragCoefficient, fluidParticleSystem, vi, currentPNeighborhood, currentPDensity);
					}

					currentPForce += currentPTmpDragForceComponent;
				}

				if (dfsphFluidConfig._useBernoulliPrinciple)
				{
					Storm::Vector3 &dynamicPressureForce = fluidParticleSystem.getTemporaryBernoulliDynamicPressureForces()[currentPIndex];
					dynamicPressureForce = computeBernoulliPrinciple(fluidParticleSystem, currentPIndex, currentPNeighborhood);
					currentPForce += dynamicPressureForce;
				}

				if (sceneSimulationConfig._noStickConstraint)
				{
					Storm::Vector3 &currentPTmpNoStickForce = fluidParticleSystem.getTemporaryNoStickForces()[currentPIndex];
					currentPTmpNoStickForce = produceNoStickConditionForces(iterationParameter, fluidParticleSystem, vi, currentPMass, currentPNeighborhood);
					currentPForce += currentPTmpNoStickForce;
				}

				if (sceneSimulationConfig._useCoandaEffect)
				{
					Storm::Vector3 &currentPTmpCoandaForce = fluidParticleSystem.getTemporaryCoandaForces()[currentPIndex];
					currentPTmpCoandaForce = produceCoandaForces(iterationParameter, fluidParticleSystem, vi, currentPMass, currentPNeighborhood);
					currentPForce += currentPTmpCoandaForce;
				}

				// We should also initialize the data field now (avoid to restart the threads).
				Storm::DFSPHSolverData &currentPDataField = dataField[currentPIndex];

				currentPDataField._nonPressureAcceleration = currentPForce / currentPMass;
				if (_useRotationFix)
				{
					currentPDataField._predictedVelocity += currentPDataField._nonPressureAcceleration * iterationParameter._deltaTime;
				}
				else
				{
					currentPDataField._predictedVelocity = vi + currentPDataField._nonPressureAcceleration * iterationParameter._deltaTime;
				}

				// Note : Maybe we should also compute a prediction of the position ?
			});
		}
	}
}

void Storm::DFSPHSolver::divergenceSolve(const Storm::IterationParameter &iterationParameter, const Storm::SceneFluidCustomDFSPHConfig &sceneDFSPHSimulationConfig, unsigned int &outIteration, float &outAverageError)
{
	Storm::ParticleSystemContainer &particleSystems = *iterationParameter._particleSystems;

	//////////////////////////////////////////////////////////////////////////
	// Init parameters
	//////////////////////////////////////////////////////////////////////////

	const float invertDeltaTime = 1.f / iterationParameter._deltaTime;

	const unsigned int minIter = sceneDFSPHSimulationConfig._minPredictIteration;
	const unsigned int maxIter = sceneDFSPHSimulationConfig._maxPredictIteration;
	const float etaCoeff = invertDeltaTime * sceneDFSPHSimulationConfig._maxDensityError * 0.01f;

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
			std::vector<Storm::Vector3> &intermediaryPressureForces = fluidPSystem.getTemporaryPressureDensityIntermediaryForces();

			const float density0 = fluidPSystem.getRestDensity();
			avgDensityErrAtom = 0.f;

			auto lambda = [&]<bool computePressureForBoundary>(Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex, const float ki)
			{
				const float currentPMass = masses[currentPIndex];
				Storm::Vector3 &currentPIntermediaryForce = intermediaryPressureForces[currentPIndex];

				const float deltaVToForce = currentPMass * invertDeltaTime;

				Storm::Vector3 &vi = currentPData._predictedVelocity;

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

						const float b_j = neighbor._notReflected ? neighborData._densityAdv : 1.f;
						const float kj = b_j * neighborData._kCoeff;

						const float kSum = ki + lastNeighborFluidSystem->getRestDensity() / density0 * kj;
						if (std::fabs(kSum) > Storm::SPHSolverPrivateLogic::k_epsilon)
						{
							const Storm::Vector3 velChange = (iterationParameter._deltaTime * kSum * lastNeighborFluidSystem->getParticleVolume()) * neighbor._gradWij;
							vi += velChange;

							const Storm::Vector3 addedPressureForce = deltaVToForce * velChange;
							currentPIntermediaryForce += addedPressureForce;
						}
					}
					else if constexpr (computePressureForBoundary)
					{
						Storm::RigidBodyParticleSystem* neighborPSystemAsBoundary = static_cast<Storm::RigidBodyParticleSystem*>(neighbor._containingParticleSystem);

						const Storm::Vector3 velChange = (iterationParameter._deltaTime * ki * neighborPSystemAsBoundary->getVolumes()[neighbor._particleIndex]) * neighbor._gradWij;	// kj already contains inverse density

						vi += velChange;

						const Storm::Vector3 addedPressureForce = deltaVToForce * velChange;
						currentPIntermediaryForce += addedPressureForce;

#if true
						if (!neighborPSystemAsBoundary->isStatic())
						{
							Storm::Vector3 &tmpPressureForce = neighborPSystemAsBoundary->getTemporaryPressureForces()[neighbor._particleIndex];
							Storm::Vector3 &tmpPressureIntermediaryForce = neighborPSystemAsBoundary->getTemporaryPressureDensityIntermediaryForces()[neighbor._particleIndex];

							std::lock_guard<std::mutex> lock{ neighbor._containingParticleSystem->_mutex };
							tmpPressureForce -= addedPressureForce;
							tmpPressureIntermediaryForce -= addedPressureForce;
						}
#endif
					}
				}
			};

			Storm::runParallel(dataFieldPair.second, [&lambda](Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex)
			{
				//////////////////////////////////////////////////////////////////////////
				// Evaluate rhs
				//////////////////////////////////////////////////////////////////////////
				const float b_i = currentPData._densityAdv;
				const float ki = b_i * currentPData._kCoeff;

				if (std::fabs(ki) > Storm::SPHSolverPrivateLogic::k_epsilon)
				{
					lambda.template operator()<true>(currentPData, currentPIndex, ki);
				}
				else
				{
					lambda.template operator()<false>(currentPData, currentPIndex, ki);
				}
			});

			//////////////////////////////////////////////////////////////////////////
			// Update rho_adv and density error
			//////////////////////////////////////////////////////////////////////////
			Storm::runParallel(dataFieldPair.second, [&](Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex)
			{
				this->computeDensityChange(iterationParameter, fluidPSystem, &dataFieldPair.second, currentPData, currentPIndex);
				avgDensityErrAtom += density0 * (currentPData._densityAdv);
			});

			avgDensityErrAtom = avgDensityErrAtom / _totalParticleCountFl;

			// Maximal allowed density fluctuation
			// use maximal density error divided by time step size
			const float eta = etaCoeff * density0;  // maxError is given in percent
			chk = chk && (avgDensityErrAtom <= eta);

			outAverageError += avgDensityErrAtom;
		}

		outAverageError /= _totalParticleCountFl;

		++outIteration;

		if (!this->shouldContinue()) STORM_UNLIKELY
		{
			return;
		}

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

void Storm::DFSPHSolver::pressureSolve(const Storm::IterationParameter &iterationParameter, const Storm::SceneFluidCustomDFSPHConfig &sceneDFSPHSimulationConfig, unsigned int &outIteration, float &outAverageError)
{
	Storm::ParticleSystemContainer &particleSystems = *iterationParameter._particleSystems;

	const unsigned int minIter = sceneDFSPHSimulationConfig._minPredictIteration;
	const unsigned int maxIter = sceneDFSPHSimulationConfig._maxPredictIteration;
	const float etaCoeff = sceneDFSPHSimulationConfig._maxPressureError * 0.01f;

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

	do
	{
		chk = true;

		outAverageError = 0.f;

		for (auto &dataFieldPair : _data)
		{
			// Since data field was made from fluid particles, no need to check.
			Storm::FluidParticleSystem &fluidPSystem = static_cast<Storm::FluidParticleSystem &>(*particleSystems.find(dataFieldPair.first)->second);

			const float density0 = fluidPSystem.getRestDensity();

			const std::vector<float> &masses = fluidPSystem.getMasses();
			const std::vector<Storm::ParticleNeighborhoodArray> &neighborhoodArrays = fluidPSystem.getNeighborhoodArrays();
			std::vector<Storm::Vector3> &temporaryVelocityPressureForces = fluidPSystem.getTemporaryPressureVelocityIntermediaryForces();
			
			auto lambda = [&]<bool computePressureForBoundary>(Storm::DFSPHSolverData & currentPData, const std::size_t currentPIndex, const float ki)
			{
				const float currentPMass = masses[currentPIndex];
				Storm::Vector3 &vi = currentPData._predictedVelocity;
				Storm::Vector3 &currentPTemporaryVelocityPressureForce = temporaryVelocityPressureForces[currentPIndex];

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

						const float b_j = neighbor._notReflected ? (neighborData._densityAdv - 1.f) : 0.f;
						const float kj = b_j * neighborData._kCoeff;
						const float kSum = ki + lastNeighborFluidSystem->getRestDensity() / density0 * kj;
						if (std::fabs(kSum) > Storm::SPHSolverPrivateLogic::k_epsilon)
						{
							const Storm::Vector3 velChange = (iterationParameter._deltaTime * kSum * lastNeighborFluidSystem->getParticleVolume()) * neighbor._gradWij;	// ki, kj already contain inverse density

							// Directly update velocities instead of storing pressure accelerations
							vi += velChange;

							currentPTemporaryVelocityPressureForce -= (currentPMass * invDeltaTime) * velChange;
						}
					}
					else if constexpr (computePressureForBoundary)
					{
						Storm::RigidBodyParticleSystem* neighborPSystemAsBoundary = static_cast<Storm::RigidBodyParticleSystem*>(neighbor._containingParticleSystem);

						// Directly update velocities instead of storing pressure accelerations
						const Storm::Vector3 velChange = (iterationParameter._deltaTime * 1.f * ki * neighborPSystemAsBoundary->getVolumes()[neighbor._particleIndex]) * neighbor._gradWij; // kj already contains inverse density

						vi += velChange;

						const Storm::Vector3 addedPressureForce = (currentPMass * invDeltaTime) * velChange;
						currentPTemporaryVelocityPressureForce += addedPressureForce;
#if true
						if (!neighborPSystemAsBoundary->isStatic())
						{

							Storm::Vector3 &tmpPressureForce = neighborPSystemAsBoundary->getTemporaryPressureForces()[neighbor._particleIndex];
							Storm::Vector3 &tmpIntermediaryPressureVelocityForce = neighborPSystemAsBoundary->getTemporaryPressureVelocityIntermediaryForces()[neighbor._particleIndex];

							std::lock_guard<std::mutex> lock{ neighbor._containingParticleSystem->_mutex };
							tmpPressureForce -= addedPressureForce;
							tmpIntermediaryPressureVelocityForce -= addedPressureForce;
						}
#endif
					}
				}
			};

			//////////////////////////////////////////////////////////////////////////
			// Compute pressure forces
			//////////////////////////////////////////////////////////////////////////
			Storm::runParallel(dataFieldPair.second, [&lambda](Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex)
			{
				//////////////////////////////////////////////////////////////////////////
				// Evaluate rhs
				//////////////////////////////////////////////////////////////////////////
				const float b_i = currentPData._densityAdv - 1.f;
				const float ki = b_i * currentPData._kCoeff;

				if (std::fabs(ki) > Storm::SPHSolverPrivateLogic::k_epsilon)
				{
					lambda.template operator()<true>(currentPData, currentPIndex, ki);
				}
				else
				{
					lambda.template operator()<false>(currentPData, currentPIndex, ki);
				}
			});

			std::atomic<float> densityError = 0.f;

			//////////////////////////////////////////////////////////////////////////
			// Update rho_adv and density error
			//////////////////////////////////////////////////////////////////////////
			Storm::runParallel(dataFieldPair.second, [&](Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex)
			{
				this->computeDensityAdv(iterationParameter, fluidPSystem, &dataFieldPair.second, currentPData, currentPIndex);
				densityError += density0 * currentPData._densityAdv - density0;
			});

			outAverageError = densityError / _totalParticleCountFl;

			// Maximal allowed density fluctuation
			const float eta = etaCoeff * density0;  // maxError is given in percent
			chk = chk && (outAverageError <= eta);
		}

		++outIteration;

		if (!this->shouldContinue()) STORM_UNLIKELY
		{
			return;
		}
	} while ((!chk || (outIteration < minIter)) && (outIteration < maxIter));
}

void Storm::DFSPHSolver::computeDFSPHFactor(const Storm::IterationParameter &iterationParameter, Storm::FluidParticleSystem &fluidPSystem, Storm::DFSPHSolver::DFSPHSolverDataArray &pSystemData, const double kMultiplicationCoeff)
{
	//////////////////////////////////////////////////////////////////////////
	// Init parameters
	//////////////////////////////////////////////////////////////////////////

	const std::vector<Storm::ParticleNeighborhoodArray> &neighborhoodArrays = fluidPSystem.getNeighborhoodArrays();

	//////////////////////////////////////////////////////////////////////////
	// Compute pressure stiffness denominator
	//////////////////////////////////////////////////////////////////////////
	Storm::runParallel(pSystemData, [&iterationParameter, &neighborhoodArrays, kMultiplicationCoeff](Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex)
	{
		const Storm::ParticleNeighborhoodArray &currentPNeighborhood = neighborhoodArrays[currentPIndex];

		//////////////////////////////////////////////////////////////////////////
		// Compute gradient dp_i/dx_j * (1/k)  and dp_j/dx_j * (1/k)
		//////////////////////////////////////////////////////////////////////////
		double sum_grad_p_k = 0.f;
		Storm::Vector3d grad_p_i = Storm::Vector3d::Zero();

		for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
		{
			if (neighbor._isFluidParticle)
			{
				const Storm::FluidParticleSystem* neighborPSystemAsFluid = static_cast<const Storm::FluidParticleSystem*>(neighbor._containingParticleSystem);
				const Storm::Vector3d grad_p_j = (-neighborPSystemAsFluid->getParticleVolume() * neighbor._gradWij).cast<double>();
				sum_grad_p_k += grad_p_j.squaredNorm();
				grad_p_i -= grad_p_j;
			}
			else
			{
				const Storm::RigidBodyParticleSystem* neighborPSystemAsBoundary = static_cast<const Storm::RigidBodyParticleSystem*>(neighbor._containingParticleSystem);
				const Storm::Vector3d grad_p_j = (-neighborPSystemAsBoundary->getVolumes()[neighbor._particleIndex] * neighbor._gradWij).cast<double>();
				grad_p_i -= grad_p_j;
			}
		}

		sum_grad_p_k += grad_p_i.squaredNorm();

		//////////////////////////////////////////////////////////////////////////
		// Compute pressure stiffness denominator
		//////////////////////////////////////////////////////////////////////////
		if (sum_grad_p_k > Storm::SPHSolverPrivateLogic::k_epsilon)
		{
			currentPData._kCoeff = static_cast<float>(kMultiplicationCoeff / sum_grad_p_k);
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
	float &densityAdv = currentPData._densityAdv;

	const float density0 = fluidPSystem.getRestDensity();

	const Storm::ParticleNeighborhoodArray &currentPNeighborhood = fluidPSystem.getNeighborhoodArrays()[currentPIndex];

	const Storm::Vector3 &vi = currentPData._predictedVelocity;

	const Storm::DFSPHSolver::DFSPHSolverDataArray* neighborDataArray = currentSystemData;
	const Storm::FluidParticleSystem* lastNeighborFluidSystem = &fluidPSystem;

	// Even though reflected particle are real particle inside the domain, they are in fact dummies to represent the notion of outside of domain particle
	// And we consider out of domain particle to be still air (or at least, we don't care about their velocity).
	// In other words, we consider reflected particle, aka out of domain particle, to have no velocity.
	const Storm::Vector3 k_velocityInCasePReflected = Storm::Vector3::Zero();
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

			const Storm::Vector3 &vj = neighbor._notReflected ? neighborData._predictedVelocity : k_velocityInCasePReflected;
			delta += lastNeighborFluidSystem->getParticleVolume() * (vi - vj).dot(neighbor._gradWij);
		}
		else
		{
			const Storm::RigidBodyParticleSystem* neighborPSystemAsBoundary = static_cast<const Storm::RigidBodyParticleSystem*>(neighbor._containingParticleSystem);
			const Storm::Vector3 &vj = neighbor._notReflected ? neighborPSystemAsBoundary->getVelocity()[neighbor._particleIndex] : k_velocityInCasePReflected;
			delta += neighborPSystemAsBoundary->getVolumes()[neighbor._particleIndex] * (vi - vj).dot(neighbor._gradWij);
		}
	}

	densityAdv = density / density0 + iterationParameter._deltaTime * delta;
	densityAdv = std::max(densityAdv, 1.f);
}

void Storm::DFSPHSolver::computeDensityChange(const Storm::IterationParameter &/*iterationParameter*/, Storm::FluidParticleSystem &fluidPSystem, const Storm::DFSPHSolver::DFSPHSolverDataArray* currentSystemData, Storm::DFSPHSolverData &currentPData, const std::size_t currentPIndex)
{
	const Storm::ParticleNeighborhoodArray &currentPNeighborhood = fluidPSystem.getNeighborhoodArrays()[currentPIndex];
	
	float &densityAdv = currentPData._densityAdv;
	densityAdv = 0.f;

	// in case of particle deficiency do not perform a divergence solve
	if (!_enableThresholdDensity || currentPNeighborhood.size() >= _neighborThresholdDensity)
	{
		const Storm::DFSPHSolver::DFSPHSolverDataArray* neighborDataArray = currentSystemData;
		const Storm::FluidParticleSystem* lastNeighborFluidSystem = &fluidPSystem;

		const Storm::Vector3 &vi = currentPData._predictedVelocity;

		// Even though reflected particle are real particle inside the domain, they are in fact dummies to represent the notion of outside of domain particle
		// And we consider out of domain particle to be still fluid (or at least, we don't care about their velocity).
		// In other words, we consider reflected particle, aka out of domain particle, to have no velocity.
		const Storm::Vector3 k_velocityInCasePReflected = Storm::Vector3::Zero();
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

				const Storm::Vector3 &vj = neighbor._notReflected ? neighborData._predictedVelocity : k_velocityInCasePReflected;
				densityAdv += lastNeighborFluidSystem->getParticleVolume() * (vi - vj).dot(neighbor._gradWij);
			}
			else
			{
				const Storm::RigidBodyParticleSystem &neighborPSystemAsBoundary = static_cast<const Storm::RigidBodyParticleSystem &>(*neighbor._containingParticleSystem);
				const Storm::Vector3 &vj = neighbor._notReflected ? neighborPSystemAsBoundary.getVelocity()[neighbor._particleIndex] : k_velocityInCasePReflected;
				densityAdv += neighborPSystemAsBoundary.getVolumes()[neighbor._particleIndex] * (vi - vj).dot(neighbor._gradWij);
			}
		}

		// only correct positive divergence
		densityAdv = std::max(densityAdv, 0.f);
	}
}
