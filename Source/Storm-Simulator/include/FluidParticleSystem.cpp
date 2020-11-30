#include "FluidParticleSystem.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"
#include "ISpacePartitionerManager.h"

#include "GeneralSimulationData.h"
#include "FluidData.h"

#include "SimulationMode.h"

#include "PartitionSelection.h"
#include "NeighborParticleReferral.h"

#include "SemiImplicitEulerSolver.h"

#include "IBlower.h"

#include "RunnerHelper.h"

#include "ParticleSystemUtils.h"


Storm::FluidParticleSystem::FluidParticleSystem(unsigned int particleSystemIndex, std::vector<Storm::Vector3> &&worldPositions) :
	Storm::ParticleSystem{ particleSystemIndex, std::move(worldPositions) }
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
	const Storm::GeneralSimulationData &generalSimulDataConfig = configMgr.getGeneralSimulationData();
	const Storm::FluidData &fluidDataConfig = configMgr.getFluidData();

	_restDensity = fluidDataConfig._density;

	const std::size_t particleCount = this->getParticleCount();

	const float particleDiameter = generalSimulDataConfig._particleRadius * 2.f;

	_particleVolume = particleDiameter * particleDiameter * particleDiameter;

	_masses.resize(particleCount, _particleVolume * _restDensity);
	_densities.resize(particleCount);
	_pressure.resize(particleCount);

	_velocityPreTimestep.resize(particleCount);
}

Storm::FluidParticleSystem::FluidParticleSystem(unsigned int particleSystemIndex, std::size_t particleCount) :
	Storm::ParticleSystem{ particleSystemIndex, particleCount }
{
	// No need to init the other thing since this constructor is only to be used in replay mode and we won't use them.
}

void Storm::FluidParticleSystem::onIterationStart()
{
	Storm::ParticleSystem::onIterationStart();

#if defined(DEBUG) || defined(_DEBUG)
	if (!configMgr.isInReplayMode())
	{
		const std::size_t particleCount = this->getParticleCount();

		assert(
			_masses.size() == particleCount &&
			_densities.size() == particleCount &&
			_velocityPreTimestep.size() == particleCount &&
			_pressure.size() == particleCount &&
			"Particle count mismatch detected! An array of particle property has not the same particle count than the other!"
		);
	}
#endif
}

void Storm::FluidParticleSystem::onSubIterationStart(const Storm::ParticleSystemContainer &allParticleSystems, const std::vector<std::unique_ptr<Storm::IBlower>> &blowers)
{
	Storm::ParticleSystem::onSubIterationStart(allParticleSystems, blowers);

	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();

	const Storm::GeneralSimulationData &generalSimulData = configMgr.getGeneralSimulationData();
	const Storm::FluidData &fluidSimulData = configMgr.getFluidData();

	const Storm::Vector3 gravityAccel = fluidSimulData._gravityEnabled ? generalSimulData._gravity : Storm::Vector3::Zero();

	Storm::runParallel(_force, [this, &gravityAccel, &blowers](Storm::Vector3 &currentPForce, const std::size_t currentPIndex)
	{
		this->internalInitializeForce(gravityAccel, blowers, currentPForce, currentPIndex);

		_velocityPreTimestep[currentPIndex] = _velocity[currentPIndex];
	});
}

bool Storm::FluidParticleSystem::isFluids() const noexcept
{
	return true;
}

bool Storm::FluidParticleSystem::isStatic() const noexcept
{
	return false;
}

bool Storm::FluidParticleSystem::isWall() const noexcept
{
	return false;
}

void Storm::FluidParticleSystem::setPositions(std::vector<Storm::Vector3> &&positions)
{
	_positions = std::move(positions);
	_isDirty = true;
}

void Storm::FluidParticleSystem::setVelocity(std::vector<Storm::Vector3> &&velocities)
{
	_velocity = std::move(velocities);
}

void Storm::FluidParticleSystem::setForces(std::vector<Storm::Vector3> &&forces)
{
	_force = std::move(forces);
}

void Storm::FluidParticleSystem::setTmpPressureForces(std::vector<Storm::Vector3> &&tmpPressureForces)
{
	_tmpPressureForce = std::move(tmpPressureForces);
}

void Storm::FluidParticleSystem::setTmpViscosityForces(std::vector<Storm::Vector3> &&tmpViscoForces)
{
	_tmpViscosityForce = std::move(tmpViscoForces);
}

float Storm::FluidParticleSystem::getRestDensity() const noexcept
{
	return _restDensity;
}

float Storm::FluidParticleSystem::getParticleVolume() const noexcept
{
	return _particleVolume;
}

std::vector<float>& Storm::FluidParticleSystem::getMasses() noexcept
{
	return _masses;
}

const std::vector<float>& Storm::FluidParticleSystem::getMasses() const noexcept
{
	return _masses;
}

std::vector<float>& Storm::FluidParticleSystem::getDensities() noexcept
{
	return _densities;
}

const std::vector<float>& Storm::FluidParticleSystem::getDensities() const noexcept
{
	return _densities;
}

std::vector<float>& Storm::FluidParticleSystem::getPressures() noexcept
{
	return _pressure;
}

const std::vector<float>& Storm::FluidParticleSystem::getPressures() const noexcept
{
	return _pressure;
}

void Storm::FluidParticleSystem::buildNeighborhoodOnParticleSystem(const Storm::ParticleSystem &otherParticleSystem, const float kernelLengthSquared)
{
	if (otherParticleSystem.getId() == this->getId())
	{
		const std::size_t particleCount = this->getParticleCount();

		Storm::runParallel(_positions, [this, kernelLengthSquared, particleCount](const Storm::Vector3 &currentParticlePosition, const std::size_t currentPIndex)
		{
			std::vector<Storm::NeighborParticleInfo> &currentNeighborhoodToFill = _neighborhood[currentPIndex];

			for (std::size_t particleIndex = 0; particleIndex < currentPIndex; ++particleIndex)
			{
				const Storm::Vector3 positionDifference = currentParticlePosition - _positions[particleIndex];
				const float vectToParticleSquaredNorm = positionDifference.squaredNorm();
				if (Storm::ParticleSystem::isElligibleNeighborParticle(kernelLengthSquared, vectToParticleSquaredNorm))
				{
					currentNeighborhoodToFill.emplace_back(this, particleIndex, positionDifference, vectToParticleSquaredNorm, true);
				}
			}

			// We would skip the current particle (the current particle shouldn't be part of its own neighborhood).

			for (std::size_t particleIndex = currentPIndex + 1; particleIndex < particleCount; ++particleIndex)
			{
				const Storm::Vector3 positionDifference = currentParticlePosition - _positions[particleIndex];
				const float vectToParticleSquaredNorm = positionDifference.squaredNorm();
				if (Storm::ParticleSystem::isElligibleNeighborParticle(kernelLengthSquared, vectToParticleSquaredNorm))
				{
					currentNeighborhoodToFill.emplace_back(this, particleIndex, positionDifference, vectToParticleSquaredNorm, true);
				}
			}
		});
	}
	else
	{
		Storm::runParallel(_positions, [this, kernelLengthSquared, &otherParticleSystem](const Storm::Vector3 &currentPPosition, const std::size_t currentPIndex)
		{
			std::vector<Storm::NeighborParticleInfo> &currentNeighborhoodToFill = _neighborhood[currentPIndex];

			const auto &otherParticleSystemPositionsArray = otherParticleSystem.getPositions();
			const std::size_t otherParticleSizeCount = otherParticleSystemPositionsArray.size();
			const bool otherParticleSystemIsFluid = otherParticleSystem.isFluids();

			for (std::size_t particleIndex = 0; particleIndex < otherParticleSizeCount; ++particleIndex)
			{
				const Storm::Vector3 positionDifference = currentPPosition - otherParticleSystemPositionsArray[particleIndex];
				const float vectToParticleSquaredNorm = positionDifference.squaredNorm();
				if (Storm::ParticleSystem::isElligibleNeighborParticle(kernelLengthSquared, vectToParticleSquaredNorm))
				{
					currentNeighborhoodToFill.emplace_back(const_cast<Storm::ParticleSystem*>(&otherParticleSystem), particleIndex, positionDifference, vectToParticleSquaredNorm, otherParticleSystemIsFluid);
				}
			}
		});
	}
}

void Storm::FluidParticleSystem::buildNeighborhoodOnParticleSystemUsingSpacePartition(const Storm::ParticleSystemContainer &allParticleSystems, const float kernelLengthSquared)
{
	const Storm::ISpacePartitionerManager &spacePartitionerMgr = Storm::SingletonHolder::instance().getSingleton<Storm::ISpacePartitionerManager>();
	Storm::runParallel(_neighborhood, [this, &allParticleSystems, kernelLengthSquared, &spacePartitionerMgr, currentSystemId = this->getId()](ParticleNeighborhoodArray &currentPNeighborhood, const std::size_t particleIndex)
	{
		const std::vector<Storm::NeighborParticleReferral>* bundleContainingPtr;
		const std::vector<Storm::NeighborParticleReferral>* outLinkedNeighborBundle[Storm::k_neighborLinkedBunkCount];

		const Storm::Vector3 &currentPPosition = _positions[particleIndex];

		// Get all particles referrals that are near the current particle position. First, get all particles inside the fluid partitioned space...
		spacePartitionerMgr.getAllBundles(bundleContainingPtr, outLinkedNeighborBundle, currentPPosition, Storm::PartitionSelection::Fluid);
		Storm::searchForNeighborhood<true, true>(
			this,
			allParticleSystems,
			kernelLengthSquared,
			currentSystemId,
			currentPNeighborhood,
			particleIndex,
			currentPPosition,
			*bundleContainingPtr,
			outLinkedNeighborBundle
		);

		spacePartitionerMgr.getAllBundles(bundleContainingPtr, outLinkedNeighborBundle, currentPPosition, Storm::PartitionSelection::StaticRigidBody);
		Storm::searchForNeighborhood<false, false>(
			this,
			allParticleSystems,
			kernelLengthSquared,
			currentSystemId,
			currentPNeighborhood,
			particleIndex,
			currentPPosition,
			*bundleContainingPtr,
			outLinkedNeighborBundle
		);

		spacePartitionerMgr.getAllBundles(bundleContainingPtr, outLinkedNeighborBundle, currentPPosition, Storm::PartitionSelection::DynamicRigidBody);
		Storm::searchForNeighborhood<false, false>(
			this,
			allParticleSystems,
			kernelLengthSquared,
			currentSystemId,
			currentPNeighborhood,
			particleIndex,
			currentPPosition,
			*bundleContainingPtr,
			outLinkedNeighborBundle
		);
	});
}

bool Storm::FluidParticleSystem::computeVelocityChange(float deltaTimeInSec, float highVelocityThresholdSquared)
{
	std::atomic<bool> highSpeed = false;

	Storm::runParallel(_force, [this, deltaTimeInSec, &highSpeed, highVelocityThresholdSquared](const Storm::Vector3 &currentForce, const std::size_t currentPIndex)
	{
		Storm::Vector3 &currentPVelocity = _velocity[currentPIndex];

		Storm::SemiImplicitEulerVelocitySolver solver{ _masses[currentPIndex], currentForce, currentPVelocity, deltaTimeInSec };

		currentPVelocity += solver._velocityVariation;
		if (currentPVelocity.squaredNorm() > highVelocityThresholdSquared)
		{
			highSpeed = true;
		}
	});

	return highSpeed;
}

void Storm::FluidParticleSystem::updatePosition(float deltaTimeInSec, bool)
{
	Storm::runParallel(_velocity, [this, deltaTimeInSec](Storm::Vector3 &currentPVelocity, const std::size_t currentPIndex)
	{
		Storm::SemiImplicitEulerPositionSolver solver{ currentPVelocity, deltaTimeInSec };

		_positions[currentPIndex] += solver._positionDisplacment;

		if (!_isDirty)
		{
			// Displacement under 0.1mm won't be considered... 
			constexpr const float k_epsilon = 0.0001f;

			if (fabs(solver._positionDisplacment.x()) > k_epsilon ||
				fabs(solver._positionDisplacment.y()) > k_epsilon ||
				fabs(solver._positionDisplacment.z()) > k_epsilon)
			{
				_isDirty = true;
			}
		}
	});
}

void Storm::FluidParticleSystem::revertToCurrentTimestep(const std::vector<std::unique_ptr<Storm::IBlower>> &blowers)
{
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const Storm::GeneralSimulationData &generalSimulData = configMgr.getGeneralSimulationData();
	const Storm::FluidData &fluidSimulData = configMgr.getFluidData();

	const Storm::Vector3 gravityAccel = fluidSimulData._gravityEnabled ? generalSimulData._gravity : Storm::Vector3::Zero();

	Storm::runParallel(_force, [this, &blowers, &gravityAccel](Storm::Vector3 &force, const std::size_t currentPIndex)
	{
		this->internalInitializeForce(gravityAccel, blowers, force, currentPIndex);
		_velocity[currentPIndex] = _velocityPreTimestep[currentPIndex];
	});
}

void Storm::FluidParticleSystem::internalInitializeForce(const Storm::Vector3 &gravityAccel, const std::vector<std::unique_ptr<Storm::IBlower>> &blowers, Storm::Vector3 &force, const std::size_t currentPIndex)
{
	const float currentPMass = _masses[currentPIndex];
	force = currentPMass * gravityAccel;

	const Storm::Vector3 &currentPPosition = _positions[currentPIndex];
	for (const std::unique_ptr<Storm::IBlower> &blowerUPtr : blowers)
	{
		blowerUPtr->applyForce(currentPPosition, force);
	}

	_tmpPressureForce[currentPIndex].setZero();
	_tmpViscosityForce[currentPIndex].setZero();
}
