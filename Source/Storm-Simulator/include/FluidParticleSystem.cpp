#include "FluidParticleSystem.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "GeneralSimulationData.h"
#include "FluidData.h"

#include "SimulationMode.h"
#include "DensitySolver.h"
#include "ViscositySolver.h"

#include "SemiImplicitEulerSolver.h"


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
}

void Storm::FluidParticleSystem::initializeIteration()
{
	Storm::ParticleSystem::initializeIteration();

#if defined(DEBUG) || defined(_DEBUG)
	const std::size_t particleCount = _densities.size();
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	switch (configMgr.getGeneralSimulationData()._simulationMode)
	{
	case Storm::SimulationMode::PCISPH:
		assert(
			particleCount == _predictedDensity.size() &&
			particleCount == _predictedPositions.size() &&
			particleCount == _pressureForce.size() &&
			"Particle count mismatch detected! An array of particle property has not the same particle count than the other!"
		);
		break;

	default:
		break;
	}
#endif
}

bool Storm::FluidParticleSystem::isFluids() const noexcept
{
	return true;
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

void Storm::FluidParticleSystem::buildNeighborhoodOnParticleSystem(const Storm::ParticleSystem &otherParticleSystem, const float kernelLengthSquared)
{
	if (otherParticleSystem.getId() == this->getId())
	{
		std::for_each(std::execution::par_unseq, std::begin(_positions), std::end(_positions), [this, kernelLengthSquared](const Storm::Vector3 &currentParticlePosition)
		{
			const std::size_t currentParticleIndex = this->getParticleIndex(_positions, currentParticlePosition);
			const std::size_t particleCount = _positions.size();

			std::vector<Storm::NeighborParticleInfo> &currentNeighborhoodToFill = _neighborhood[currentParticleIndex];
			currentNeighborhoodToFill.clear();

			for (std::size_t particleIndex = 0; particleIndex < currentParticleIndex; ++particleIndex)
			{
				const Storm::Vector3 positionDifference = currentParticlePosition - _positions[particleIndex];
				const float vectToParticleSquaredNorm = positionDifference.squaredNorm();
				if (vectToParticleSquaredNorm < kernelLengthSquared)
				{
					currentNeighborhoodToFill.emplace_back(this, particleIndex, positionDifference, vectToParticleSquaredNorm);
				}
			}

			// We would skip the current particle (the current particle shouldn't be part of its own neighborhood).

			for (std::size_t particleIndex = currentParticleIndex + 1; particleIndex < particleCount; ++particleIndex)
			{
				const Storm::Vector3 positionDifference = currentParticlePosition - _positions[particleIndex];
				const float vectToParticleSquaredNorm = positionDifference.squaredNorm();
				if (vectToParticleSquaredNorm < kernelLengthSquared)
				{
					currentNeighborhoodToFill.emplace_back(this, particleIndex, positionDifference, vectToParticleSquaredNorm);
				}
			}
		});
	}
	else
	{
		std::for_each(std::execution::par_unseq, std::begin(_positions), std::end(_positions), [this, kernelLengthSquared, &otherParticleSystem](const Storm::Vector3 &currentParticlePosition)
		{
			std::vector<Storm::NeighborParticleInfo> &currentNeighborhoodToFill = _neighborhood[this->getParticleIndex(_positions, currentParticlePosition)];
			currentNeighborhoodToFill.clear();

			const auto &otherParticleSystemPositionsArray = otherParticleSystem.getPositions();
			const std::size_t otherParticleSizeCount = otherParticleSystemPositionsArray.size();

			for (std::size_t particleIndex = 0; particleIndex < otherParticleSizeCount; ++particleIndex)
			{
				const Storm::Vector3 positionDifference = currentParticlePosition - otherParticleSystemPositionsArray[particleIndex];
				const float vectToParticleSquaredNorm = positionDifference.squaredNorm();
				if (vectToParticleSquaredNorm < kernelLengthSquared)
				{
					currentNeighborhoodToFill.emplace_back(&otherParticleSystem, particleIndex, positionDifference, vectToParticleSquaredNorm);
				}
			}
		});
	}
}

void Storm::FluidParticleSystem::executePCISPH()
{
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const Storm::Vector3 &gravity = configMgr.getGeneralSimulationData()._gravity;

	std::for_each(std::execution::par_unseq, std::begin(_force), std::end(_force), [this, &gravity](Storm::Vector3 &force)
	{
		// External force
		const std::size_t iter = this->getParticleIndex(_force, force);
		force += _massPerParticle * gravity;

		// Density
		_densities[iter] = Storm::DensitySolver::computeDensityPCISPH(_massPerParticle, _neighborhood[iter]);
	});

	// Viscosity
	std::for_each(std::execution::par_unseq, std::begin(_force), std::end(_force), [this](Storm::Vector3 &force)
	{
		const std::size_t iter = this->getParticleIndex(_force, force);
		force += Storm::ViscositySolver::computeViscosityForcePCISPH(_massPerParticle, _densities[iter], _velocity[iter], _neighborhood[iter]);
	});
}

void Storm::FluidParticleSystem::updatePosition(float deltaTimeInSec)
{
	std::for_each(std::execution::par_unseq, std::begin(_force), std::end(_force), [this, deltaTimeInSec](const Storm::Vector3 &currentForce)
	{
		const std::size_t currentParticleIndex = this->getParticleIndex(_force, currentForce);

		Storm::SemiImplicitEulerSolver solver{ _massPerParticle, currentForce, _velocity[currentParticleIndex], deltaTimeInSec };

		_velocity[currentParticleIndex] += solver._velocityVariation;
		_positions[currentParticleIndex] += solver._positionDisplacment;

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
