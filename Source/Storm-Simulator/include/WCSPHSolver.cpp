#include "WCSPHSolver.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"
#include "SimulatorManager.h"

#include "GeneralSimulationData.h"
#include "FluidData.h"

#include "FluidParticleSystem.h"
#include "RigidBodyParticleSystem.h"

#include "IterationParameter.h"

#include "ParticleSelector.h"

#include "Kernel.h"

#include "RunnerHelper.h"


void Storm::WCSPHSolver::execute(const Storm::IterationParameter &iterationParameter)
{
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const Storm::GeneralSimulationData &generalSimulData = configMgr.getGeneralSimulationData();
	const Storm::FluidData &fluidConfigData = configMgr.getFluidData();

	Storm::SimulatorManager &simulMgr = Storm::SimulatorManager::instance();

	const float k_kernelZero = Storm::retrieveKernelZeroValue(generalSimulData._kernelMode);
	const Storm::RawKernelMethodDelegate rawKernel = Storm::retrieveRawKernelMethod(generalSimulData._kernelMode);
	const Storm::GradKernelMethodDelegate gradKernel = Storm::retrieveGradKernelMethod(generalSimulData._kernelMode);

	Storm::ParticleSystemContainer &particleSystems = *iterationParameter._particleSystems;

	// 1st : Initialize iteration
	simulMgr.advanceBlowersTime(iterationParameter._deltaTime);
	simulMgr.refreshParticleNeighborhood();
	simulMgr.subIterationStart();

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

	const float k_kernelLengthSquared = iterationParameter._kernelLength * iterationParameter._kernelLength;

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
			const std::vector<float> &masses = fluidParticleSystem.getMasses();
			const std::vector<float> &densities = fluidParticleSystem.getDensities();
			const std::vector<float> &pressures = fluidParticleSystem.getPressures();
			const std::vector<Storm::Vector3> &velocities = fluidParticleSystem.getVelocity();
			std::vector<Storm::Vector3> &temporaryPPressureForce = fluidParticleSystem.getTemporaryPressureForces();
			std::vector<Storm::Vector3> &temporaryPViscoForce = fluidParticleSystem.getTemporaryViscosityForces();

			Storm::runParallel(fluidParticleSystem.getForces(), [&](Storm::Vector3 &currentPForce, const std::size_t currentPIndex)
			{
				const float currentPMass = masses[currentPIndex];
				const float currentPDensity = densities[currentPIndex];
				const float currentPPressure = pressures[currentPIndex];
				const Storm::Vector3 &vi = velocities[currentPIndex];

				const float currentPFluidPressureCoeff = currentPPressure / (currentPDensity * currentPDensity);

				const float restMassDensity = currentPMass * density0;
				const float currentPRestMassDensityBoundaryPressureCoeff = restMassDensity * (currentPFluidPressureCoeff + (currentPPressure / density0Squared));

				const float viscoPrecoeff = 0.01f * k_kernelLengthSquared;

				Storm::Vector3 totalPressureForceOnParticle = Storm::Vector3::Zero();
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

						// Pressure
						const float neighborPressureCoeff = neighborPSystemAsFluid->getPressures()[neighbor._particleIndex] / (neighborDensity * neighborDensity);
						pressureComponent = -(restMassDensity * neighborVolume * (currentPFluidPressureCoeff + neighborPressureCoeff)) * gradKernel_NablaWij;

						// Viscosity
						viscosityComponent = (viscoGlobalCoeff * fluidConfigData._dynamicViscosity * neighborMass / neighborRawDensity) * gradKernel_NablaWij;
					}
					else
					{
						const Storm::RigidBodyParticleSystem* neighborPSystemAsBoundary = static_cast<Storm::RigidBodyParticleSystem*>(neighbor._containingParticleSystem);

						const float neighborVolume = neighborPSystemAsBoundary->getVolumes()[neighbor._particleIndex];
						const float rbViscosity = neighborPSystemAsBoundary->getViscosity();

						// Pressure
						pressureComponent = -(currentPRestMassDensityBoundaryPressureCoeff * neighborVolume) * gradKernel_NablaWij;

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
							boundaryNeighborTmpPressureForce -= pressureComponent;
							boundaryNeighborTmpViscosityForce -= viscosityComponent;
						}
					}

					totalPressureForceOnParticle += pressureComponent;
					totalViscosityForceOnParticle += viscosityComponent;
				}

				currentPForce += totalPressureForceOnParticle;
				currentPForce += totalViscosityForceOnParticle;

				temporaryPPressureForce[currentPIndex] = totalPressureForceOnParticle;
				temporaryPViscoForce[currentPIndex] = totalViscosityForceOnParticle;
			});
		}
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

				currentPVelocity += currentPForce / currentPMass * iterationParameter._deltaTime;
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
