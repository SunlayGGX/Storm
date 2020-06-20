#include "FluidParticleSystem.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "GeneralSimulationData.h"
#include "FluidData.h"
#include "DensitySolver.h"



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

}

bool Storm::FluidParticleSystem::isFluids() const noexcept
{
	return true;
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
				const float vectToParticleSquaredNorm = (currentParticlePosition - _positions[particleIndex]).squaredNorm();
				if (vectToParticleSquaredNorm < kernelLengthSquared)
				{
					currentNeighborhoodToFill.emplace_back(this, particleIndex, vectToParticleSquaredNorm);
				}
			}

			// We would skip the current particle (the current particle shouldn't be part of its own neighborhood).

			for (std::size_t particleIndex = currentParticleIndex + 1; particleIndex < particleCount; ++particleIndex)
			{
				const float vectToParticleSquaredNorm = (currentParticlePosition - _positions[particleIndex]).squaredNorm();
				if (vectToParticleSquaredNorm < kernelLengthSquared)
				{
					currentNeighborhoodToFill.emplace_back(this, particleIndex, vectToParticleSquaredNorm);
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
				const float vectToParticleSquaredNorm = (currentParticlePosition - otherParticleSystemPositionsArray[particleIndex]).squaredNorm();
				if (vectToParticleSquaredNorm < kernelLengthSquared)
				{
					currentNeighborhoodToFill.emplace_back(&otherParticleSystem, particleIndex, vectToParticleSquaredNorm);
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
}

void Storm::FluidParticleSystem::updatePosition(float deltaTimeInSec)
{
	std::for_each(std::execution::par_unseq, std::begin(_force), std::end(_force), [this, deltaTimeInSec](const Storm::Vector3 &currentForce)
	{
		const std::size_t currentParticleIndex = this->getParticleIndex(_force, currentForce);

		Storm::Vector3 &currentAccel = _accelerations[currentParticleIndex];
		currentAccel = currentForce / _massPerParticle;

		Storm::Vector3 &currentVelocity = _velocity[currentParticleIndex];
		currentVelocity += deltaTimeInSec * currentAccel;

		const Storm::Vector3 displacmentPosition = deltaTimeInSec * currentVelocity;
		_positions[currentParticleIndex] += displacmentPosition;

		if (!_isDirty)
		{
			// Displacement under 0.1mm won't be considered... 
			constexpr const float k_epsilon = 0.0001f;

			_isDirty = 
				fabs(displacmentPosition.x()) > k_epsilon ||
				fabs(displacmentPosition.y()) > k_epsilon ||
				fabs(displacmentPosition.z()) > k_epsilon
				;
		}
	});
}
