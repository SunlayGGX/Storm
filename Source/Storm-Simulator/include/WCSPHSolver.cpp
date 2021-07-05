#include "WCSPHSolver.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"
#include "SimulatorManager.h"

#include "SceneSimulationConfig.h"
#include "SceneFluidConfig.h"

#include "FluidParticleSystem.h"
#include "RigidBodyParticleSystem.h"

#include "IterationParameter.h"

#include "ParticleSelector.h"

#include "Kernel.h"
#include "ViscosityMethod.h"

#include "RunnerHelper.h"


namespace
{
	enum DragComputeMode
	{
		NoCompute,
		RbOnly,
		ForAll,
	};

	template<Storm::ViscosityMethod viscosityMethodOnFluid, Storm::ViscosityMethod viscosityMethodOnRigidBody, DragComputeMode dragComputeMode>
	void computeAll(const Storm::IterationParameter &iterationParameter, const Storm::SceneFluidConfig &fluidConfig, const float density0, const float currentPMass, const Storm::Vector3 &vi, const Storm::ParticleNeighborhoodArray &currentPNeighborhood, const float currentPDensity, const float currentPPressure, const float viscoPrecoeff, Storm::Vector3 &outTotalPressureForceOnParticle, Storm::Vector3 &outTotalViscosityForceOnParticle, Storm::Vector3 &outTotalDragForceOnParticle)
	{
		outTotalPressureForceOnParticle = Storm::Vector3::Zero();
		outTotalViscosityForceOnParticle = Storm::Vector3::Zero();
		outTotalDragForceOnParticle = Storm::Vector3::Zero();

		const float currentPFluidPressureCoeff = currentPPressure / (currentPDensity * currentPDensity);

		const float restMassDensity = currentPMass * density0;
		const float currentPRestMassDensityBoundaryPressureCoeff = restMassDensity * (currentPFluidPressureCoeff + (currentPPressure / (density0 * density0)));

		const float dragPreCoeff = -fluidConfig._uniformDragCoefficient * currentPDensity;

		for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
		{
			const Storm::Vector3 vij = vi - neighbor._containingParticleSystem->getVelocity()[neighbor._particleIndex];

			const float vijDotXij = vij.dot(neighbor._xij);
			const float viscoGlobalCoeff = currentPMass * 10.f * vijDotXij / (neighbor._xijSquaredNorm + viscoPrecoeff);

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

				// Pressure
				const float neighborPressureCoeff = neighborPSystemAsFluid->getPressures()[neighbor._particleIndex] / (neighborDensity * neighborDensity);
				pressureComponent = -(restMassDensity * neighborVolume * (currentPFluidPressureCoeff + neighborPressureCoeff)) * neighbor._gradWij;

				// Viscosity
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

				// Drag
				if constexpr (dragComputeMode == DragComputeMode::ForAll)
				{
					outTotalDragForceOnParticle += (dragPreCoeff * neighbor._Wij * vij.norm()) * vij;
				}
			}
			else
			{
				const Storm::RigidBodyParticleSystem* neighborPSystemAsBoundary = static_cast<Storm::RigidBodyParticleSystem*>(neighbor._containingParticleSystem);

				const float neighborVolume = neighborPSystemAsBoundary->getVolumes()[neighbor._particleIndex];

				// Pressure
				pressureComponent = -(currentPRestMassDensityBoundaryPressureCoeff * neighborVolume) * neighbor._gradWij;

				// Viscosity
				const float rbViscosity = neighborPSystemAsBoundary->getViscosity();
				if (rbViscosity > 0.f)
				{
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

				//Drag
				Storm::Vector3 dragComponent;
				if constexpr (dragComputeMode != DragComputeMode::NoCompute)
				{
					dragComponent = (dragPreCoeff * neighbor._Wij * vij.norm()) * vij;
					outTotalDragForceOnParticle += dragComponent;
				}
				else
				{
					dragComponent = Storm::Vector3::Zero();
				}

				// Mirror the force on the boundary solid following the 3rd newton law
				if (!neighborPSystemAsBoundary->isStatic())
				{
					Storm::Vector3 &boundaryNeighborForce = neighbor._containingParticleSystem->getForces()[neighbor._particleIndex];
					Storm::Vector3 &boundaryNeighborTmpPressureForce = neighbor._containingParticleSystem->getTemporaryPressureForces()[neighbor._particleIndex];
					Storm::Vector3 &boundaryNeighborTmpViscosityForce = neighbor._containingParticleSystem->getTemporaryViscosityForces()[neighbor._particleIndex];
					Storm::Vector3 &boundaryNeighborTmpDragForce = neighbor._containingParticleSystem->getTemporaryDragForces()[neighbor._particleIndex];

#define STORM_ENABLE_PRESSURE_FORCE_ON_RB true

					const Storm::Vector3 sumForces =
#if STORM_ENABLE_PRESSURE_FORCE_ON_RB
						pressureComponent +
#endif
						viscosityComponent +
						dragComponent
						;

					std::lock_guard<std::mutex> lock{ neighbor._containingParticleSystem->_mutex };
					boundaryNeighborForce -= sumForces;
#if STORM_ENABLE_PRESSURE_FORCE_ON_RB
					boundaryNeighborTmpPressureForce -= pressureComponent;
#endif
					boundaryNeighborTmpViscosityForce -= viscosityComponent;

					if constexpr (dragComputeMode != DragComputeMode::NoCompute)
					{
						boundaryNeighborTmpDragForce -= dragComponent;
					}
				}
			}

			outTotalPressureForceOnParticle += pressureComponent;
			outTotalViscosityForceOnParticle += viscosityComponent;
		}
	}
}


void Storm::WCSPHSolver::execute(const Storm::IterationParameter &iterationParameter)
{
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const Storm::SceneSimulationConfig &sceneSimulationConfig = configMgr.getSceneSimulationConfig();
	const Storm::SceneFluidConfig &fluidConfig = configMgr.getSceneFluidConfig();

	Storm::SimulatorManager &simulMgr = Storm::SimulatorManager::instance();

	const float k_kernelZero = Storm::retrieveKernelZeroValue(sceneSimulationConfig._kernelMode);

	Storm::ParticleSystemContainer &particleSystems = *iterationParameter._particleSystems;

	// 1st : Initialize iteration
	simulMgr.advanceBlowersTime(iterationParameter._deltaTime);
	simulMgr.refreshParticleNeighborhood();
	simulMgr.subIterationStart();

	if (!this->shouldContinue()) STORM_UNLIKELY
	{
		return;
	}

	// 2nd : compute densities and pressure data
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
			std::vector<float> &masses = fluidParticleSystem.getMasses();

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

				// Pressure
				float &currentPPressure = pressures[currentPIndex];
				if (currentPDensity < density0)
				{
					currentPDensity = density0;
					currentPPressure = 0.f;
				}
				else
				{
					currentPPressure = fluidConfig._kPressureStiffnessCoeff * (std::powf(currentPDensity / density0, fluidConfig._kPressureExponentCoeff) - 1.f);
				}

				float &currentPMass = masses[currentPIndex];
				currentPMass = currentPDensity * particleVolume;
			});
		}
	}

	if (!this->shouldContinue()) STORM_UNLIKELY
	{
		return;
	}

	const float k_kernelLengthSquared = iterationParameter._kernelLength * iterationParameter._kernelLength;
	const float k_kernelLengthSquared00_1 = 0.01f * k_kernelLengthSquared;

	// 3rd : Compute forces : pressure and viscosity
	for (auto &particleSystemPair : particleSystems)
	{
		Storm::ParticleSystem &currentParticleSystem = *particleSystemPair.second;
		if (currentParticleSystem.isFluids())
		{
			Storm::FluidParticleSystem &fluidParticleSystem = static_cast<Storm::FluidParticleSystem &>(currentParticleSystem);

			const float density0 = fluidParticleSystem.getRestDensity();
			const float density0Squared = density0 * density0;
			const std::vector<Storm::ParticleNeighborhoodArray> &neighborhoodArrays = currentParticleSystem.getNeighborhoodArrays();
			std::vector<float> &masses = fluidParticleSystem.getMasses();
			const std::vector<float> &densities = fluidParticleSystem.getDensities();
			const std::vector<float> &pressures = fluidParticleSystem.getPressures();
			const std::vector<Storm::Vector3> &velocities = fluidParticleSystem.getVelocity();
			std::vector<Storm::Vector3> &temporaryPPressureForce = fluidParticleSystem.getTemporaryPressureForces();
			std::vector<Storm::Vector3> &temporaryPViscoForce = fluidParticleSystem.getTemporaryViscosityForces();
			std::vector<Storm::Vector3> &temporaryPDragForce = fluidParticleSystem.getTemporaryDragForces();

			Storm::runParallel(fluidParticleSystem.getForces(), [&](Storm::Vector3 &currentPForce, const std::size_t currentPIndex)
			{
				Storm::Vector3 &currentPTmpPressureForce = temporaryPPressureForce[currentPIndex];
				Storm::Vector3 &currentPTmpViscoForce = temporaryPViscoForce[currentPIndex];
				Storm::Vector3 &currentPTmpDragForce = temporaryPDragForce[currentPIndex];

#define STORM_COMPUTE_ALL(fluidMethod, rbMethod)	\
	if (fluidConfig._uniformDragCoefficient == 0.f)	\
	{												\
		computeAll<fluidMethod, rbMethod, DragComputeMode::NoCompute>(iterationParameter, fluidConfig, fluidParticleSystem.getRestDensity(), masses[currentPIndex], velocities[currentPIndex], neighborhoodArrays[currentPIndex], densities[currentPIndex], pressures[currentPIndex], k_kernelLengthSquared00_1, currentPTmpPressureForce, currentPTmpViscoForce, currentPTmpDragForce);					  \
	}												\
	else if (fluidConfig._applyDragEffectOnFluid)	\
	{												\
		computeAll<fluidMethod, rbMethod, DragComputeMode::ForAll>(iterationParameter, fluidConfig, fluidParticleSystem.getRestDensity(), masses[currentPIndex], velocities[currentPIndex], neighborhoodArrays[currentPIndex], densities[currentPIndex], pressures[currentPIndex], k_kernelLengthSquared00_1, currentPTmpPressureForce, currentPTmpViscoForce, currentPTmpDragForce);					  \
	}												\
	else											\
	{												\
		computeAll<fluidMethod, rbMethod, DragComputeMode::RbOnly>(iterationParameter, fluidConfig, fluidParticleSystem.getRestDensity(), masses[currentPIndex], velocities[currentPIndex], neighborhoodArrays[currentPIndex], densities[currentPIndex], pressures[currentPIndex], k_kernelLengthSquared00_1, currentPTmpPressureForce, currentPTmpViscoForce, currentPTmpDragForce);					  \
	}


				switch (sceneSimulationConfig._fluidViscoMethod)
				{
				case Storm::ViscosityMethod::Standard:
					switch (sceneSimulationConfig._rbViscoMethod)
					{
					case Storm::ViscosityMethod::Standard:
						STORM_COMPUTE_ALL(Storm::ViscosityMethod::Standard, Storm::ViscosityMethod::Standard);
						break;
					case Storm::ViscosityMethod::XSPH:
						STORM_COMPUTE_ALL(Storm::ViscosityMethod::Standard, Storm::ViscosityMethod::XSPH);
						break;

					default:
						Storm::throwException<Storm::Exception>("Non implemented viscosity method to use on rigid body!");
					}
					break;

				case Storm::ViscosityMethod::XSPH:
					switch (sceneSimulationConfig._rbViscoMethod)
					{
					case Storm::ViscosityMethod::Standard:
						STORM_COMPUTE_ALL(Storm::ViscosityMethod::XSPH, Storm::ViscosityMethod::Standard);
						break;
					case Storm::ViscosityMethod::XSPH:
						STORM_COMPUTE_ALL(Storm::ViscosityMethod::XSPH, Storm::ViscosityMethod::XSPH);
						break;

					default:
						Storm::throwException<Storm::Exception>("Non implemented viscosity method to use on rigid body!");
					}

				default:
					Storm::throwException<Storm::Exception>("Non implemented viscosity method to use on fluid!");
				}

#undef STORM_COMPUTE_VISCOSITY_AND_PRESSURE

				currentPForce += currentPTmpPressureForce;
				currentPForce += currentPTmpViscoForce;
			});
		}
	}

	if (!this->shouldContinue()) STORM_UNLIKELY
	{
		return;
	}

	// 4th : flush physics state (rigid bodies)
	simulMgr.flushPhysics(iterationParameter._deltaTime);

	// 5th : update fluid positions (Euler integration)
	std::atomic<bool> dirtyTmp;
	for (auto &particleSystemPair : particleSystems)
	{
		Storm::ParticleSystem &currentPSystem = *particleSystemPair.second;
		if (currentPSystem.isFluids())
		{
			Storm::FluidParticleSystem &currentPSystemAsFluid = static_cast<Storm::FluidParticleSystem &>(currentPSystem);
			const std::vector<float> &masses = currentPSystemAsFluid.getMasses();
			std::vector<Storm::Vector3> &velocities = currentPSystemAsFluid.getVelocity();
			std::vector<Storm::Vector3> &positions = currentPSystemAsFluid.getPositions();
			const std::vector<Storm::Vector3> &force = currentPSystemAsFluid.getForces();

			dirtyTmp = false;
			constexpr const float minForceDirtyEpsilon = 0.0001f;

			Storm::runParallel(force, [&](const Storm::Vector3 &currentPForce, const std::size_t currentPIndex) 
			{
				const float &currentPMass = masses[currentPIndex];
				Storm::Vector3 &currentPVelocity = velocities[currentPIndex];
				Storm::Vector3 &currentPPositions = positions[currentPIndex];

				const float forceToVelocityCoeff = iterationParameter._deltaTime / currentPMass;
				currentPVelocity += currentPForce * forceToVelocityCoeff;
				currentPPositions += currentPVelocity * iterationParameter._deltaTime;

				if (!dirtyTmp)
				{
					if (
						std::fabs(currentPForce.x()) > minForceDirtyEpsilon ||
						std::fabs(currentPForce.y()) > minForceDirtyEpsilon ||
						std::fabs(currentPForce.z()) > minForceDirtyEpsilon
						)
					{
						dirtyTmp = true;
					}
				}
			});

			currentPSystemAsFluid.setIsDirty(dirtyTmp);
		}
	}
}

void Storm::WCSPHSolver::removeRawEndData(const unsigned int pSystemId, std::size_t toRemoveCount)
{

}
