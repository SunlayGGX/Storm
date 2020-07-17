#include "FluidParticleSystem.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "GeneralSimulationData.h"
#include "FluidData.h"

#include "SimulationMode.h"

#include "SemiImplicitEulerSolver.h"

#include "RunnerHelper.h"


namespace
{
	float computeDefaultFluidParticleMass()
	{
		const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();

		const float fluidDensity = configMgr.getFluidData()._density;

		return fluidDensity * Storm::ParticleSystem::computeParticleDefaultVolume();
	}
}


Storm::FluidParticleSystem::FluidParticleSystem(unsigned int particleSystemIndex, std::vector<Storm::Vector3> &&worldPositions) :
	Storm::ParticleSystem{ particleSystemIndex, std::move(worldPositions), computeDefaultFluidParticleMass() }
{
	const std::size_t particleCount = _positions.size();

	_pressures.resize(particleCount);

	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	switch (configMgr.getGeneralSimulationData()._simulationMode)
	{
	case Storm::SimulationMode::PCISPH:
		_predictedDensity.resize(particleCount);
		_predictedPositions.resize(particleCount);
		_pressureForce.resize(particleCount);
		break;

	default:
		break;
	}

	_restDensity = configMgr.getFluidData()._density;
}

void Storm::FluidParticleSystem::initializeIteration()
{
	Storm::ParticleSystem::initializeIteration();
	
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const Storm::GeneralSimulationData &generalSimulData = configMgr.getGeneralSimulationData();
	const auto currentSimulationMode = generalSimulData._simulationMode;

	bool usePredictedArrays;
	switch (currentSimulationMode)
	{
	case Storm::SimulationMode::PCISPH:
		usePredictedArrays = true;
		break;

	default:
		usePredictedArrays = false;
		break;
	}

#if defined(DEBUG) || defined(_DEBUG)
	const std::size_t particleCount = _densities.size();
	assert(
		particleCount == _pressures.size() &&
		"Particle count mismatch detected! An array of particle property has not the same particle count than the other!"
	);

	if (usePredictedArrays)
	{
		assert(
			particleCount == _predictedDensity.size() &&
			particleCount == _predictedPositions.size() &&
			particleCount == _pressureForce.size() &&
			"Particle count mismatch detected! An array of particle property has not the same particle count than the other!"
		);
	}
	
#endif

	const Storm::Vector3 &gravityAccel = generalSimulData._gravity;

	if (usePredictedArrays)
	{
		Storm::runParallel(_force, [this, usePredictedArrays, &gravityAccel](Storm::Vector3 &currentPForce, const std::size_t currentPIndex)
		{
			const float currentPMass = _masses[currentPIndex];
			currentPForce = currentPMass * gravityAccel;

			_pressureForce[currentPIndex].setZero();
		});
	}
	else
	{
		Storm::runParallel(_force, [this, usePredictedArrays, &gravityAccel](Storm::Vector3 &currentPForce, const std::size_t currentPIndex)
		{
			const float currentPMass = _masses[currentPIndex];
			currentPForce = currentPMass * gravityAccel;;
		});
	}
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

const std::vector<float>& Storm::FluidParticleSystem::getPredictedDensities() const noexcept
{
	return _predictedDensity;
}

std::vector<float>& Storm::FluidParticleSystem::getPredictedDensities() noexcept
{
	return _predictedDensity;
}

const std::vector<Storm::Vector3>& Storm::FluidParticleSystem::getPredictedPositions() const noexcept
{
	return _predictedPositions;
}

std::vector<Storm::Vector3>& Storm::FluidParticleSystem::getPredictedPositions() noexcept
{
	return _predictedPositions;
}

const std::vector<Storm::Vector3>& Storm::FluidParticleSystem::getPredictedPressureForces() const noexcept
{
	return _pressureForce;
}

std::vector<Storm::Vector3>& Storm::FluidParticleSystem::getPredictedPressureForces() noexcept
{
	return _pressureForce;
}

const std::vector<float>& Storm::FluidParticleSystem::getPressures() const noexcept
{
	return _pressures;
}

std::vector<float>& Storm::FluidParticleSystem::getPressures() noexcept
{
	return _pressures;
}

const std::vector<float>& Storm::FluidParticleSystem::getVolumes() const
{
	Storm::throwException<std::logic_error>(__FUNCTION__ " shouldn't be called for fluids particle system (only implemented for rigidbodies)");
}

std::vector<float>& Storm::FluidParticleSystem::getVolumes()
{
	Storm::throwException<std::logic_error>(__FUNCTION__ " shouldn't be called for fluids particle system (only implemented for rigidbodies)");
}

float Storm::FluidParticleSystem::getParticleVolume() const noexcept
{
	return _particleVolume;
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

void Storm::FluidParticleSystem::flushPressureToTotalForce()
{
	Storm::runParallel(_force, [this](Storm::Vector3 &currentPForce, const std::size_t currentPIndex)
	{
		currentPForce += _pressureForce[currentPIndex];
	});
}
