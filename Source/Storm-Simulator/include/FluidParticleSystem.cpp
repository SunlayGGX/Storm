#include "FluidParticleSystem.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "GeneralSimulationData.h"
#include "FluidData.h"

#include "SimulationMode.h"

#include "SemiImplicitEulerSolver.h"

#include "RunnerHelper.h"


Storm::FluidParticleSystem::FluidParticleSystem(unsigned int particleSystemIndex, std::vector<Storm::Vector3> &&worldPositions) :
	Storm::ParticleSystem{ particleSystemIndex, std::move(worldPositions) }
{
	const std::size_t particleCount = _positions.size();

	_masses.resize(particleCount);
}

void Storm::FluidParticleSystem::initializeIteration()
{
	Storm::ParticleSystem::initializeIteration();

#if defined(DEBUG) || defined(_DEBUG)
	const std::size_t particleCount = _positions.size();

	assert(
		_masses.size() == particleCount &&
		"Particle count mismatch detected! An array of particle property has not the same particle count than the other!"
	);
#endif

	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const Storm::GeneralSimulationData &generalSimulData = configMgr.getGeneralSimulationData();

	const Storm::Vector3 &gravityAccel = generalSimulData._gravity;

	Storm::runParallel(_force, [this, &gravityAccel](Storm::Vector3 &currentPForce, const std::size_t currentPIndex)
	{
		const float currentPMass = _masses[currentPIndex];
		currentPForce = currentPMass * gravityAccel;
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

float Storm::FluidParticleSystem::getRestDensity() const noexcept
{
	return _restDensity;
}

void Storm::FluidParticleSystem::buildNeighborhoodOnParticleSystem(const Storm::ParticleSystem &otherParticleSystem, const float kernelLengthSquared)
{
	if (otherParticleSystem.getId() == this->getId())
	{
		Storm::runParallel(_positions, [this, kernelLengthSquared](const Storm::Vector3 &currentParticlePosition, const std::size_t currentPIndex)
		{
			const std::size_t particleCount = _positions.size();

			std::vector<Storm::NeighborParticleInfo> &currentNeighborhoodToFill = _neighborhood[currentPIndex];
			currentNeighborhoodToFill.clear();

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
			currentNeighborhoodToFill.clear();

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

void Storm::FluidParticleSystem::updatePosition(float deltaTimeInSec)
{
	Storm::runParallel(_force, [this, deltaTimeInSec](const Storm::Vector3 &currentForce, const std::size_t currentPIndex)
	{
		Storm::Vector3 &currentPVelocity = _velocity[currentPIndex];

		Storm::SemiImplicitEulerSolver solver{ _masses[currentPIndex], currentForce, currentPVelocity, deltaTimeInSec };

		currentPVelocity += solver._velocityVariation;
		_positions[currentPIndex] += solver._positionDisplacment;

		if (!_isDirty)
		{
			// Displacement under 0.1mm won't be considered... 
			constexpr const float k_epsilon = 0.0001f;

			_isDirty = 
				fabs(solver._positionDisplacment.x()) > k_epsilon ||
				fabs(solver._positionDisplacment.y()) > k_epsilon ||
				fabs(solver._positionDisplacment.z()) > k_epsilon
				;
		}
	});
}
