#include "FluidParticleSystem.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "GeneralSimulationData.h"
#include "FluidData.h"



namespace
{
	float computeDefaultFluidParticleMass()
	{
		const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();

		const float particleRadius = configMgr.getGeneralSimulationData()._particleRadius;
		const float fluidDensity = configMgr.getFluidData()._density;

		const float particleVolume = particleRadius * particleRadius * particleRadius;

		return fluidDensity * particleVolume;
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
	const unsigned int otherParticleSystemId = otherParticleSystem.getId();
	if (otherParticleSystemId == this->getId())
	{
		std::for_each(std::execution::par_unseq, std::begin(_positions), std::end(_positions), [this, kernelLengthSquared](const Storm::Vector3 &currentParticlePosition)
		{
			const std::size_t currentParticleIndex = this->getParticleIndex(_positions, currentParticlePosition);
			const std::size_t particleCount = _positions.size();

			std::vector<Storm::ParticleIdentifier> &currentNeighborhoodToFill = _neighborhood[currentParticleIndex];
			currentNeighborhoodToFill.clear();

			for (std::size_t particleIndex = 0; particleIndex < currentParticleIndex; ++particleIndex)
			{
				if ((currentParticlePosition - _positions[particleIndex]).squaredNorm() < kernelLengthSquared)
				{
					currentNeighborhoodToFill.emplace_back(_particleSystemIndex, particleIndex);
				}
			}

			for (std::size_t particleIndex = currentParticleIndex + 1; particleIndex < particleCount; ++particleIndex)
			{
				if ((currentParticlePosition - _positions[particleIndex]).squaredNorm() < kernelLengthSquared)
				{
					currentNeighborhoodToFill.emplace_back(_particleSystemIndex, particleIndex);
				}
			}
		});
	}
	else
	{
		std::for_each(std::execution::par_unseq, std::begin(_positions), std::end(_positions), [this, kernelLengthSquared, &otherParticleSystem, otherParticleSystemId](const Storm::Vector3 &currentParticlePosition)
		{
			std::vector<Storm::ParticleIdentifier> &currentNeighborhoodToFill = _neighborhood[this->getParticleIndex(_positions, currentParticlePosition)];
			currentNeighborhoodToFill.clear();

			const auto &otherParticleSystemPositionsArray = otherParticleSystem.getPositions();
			const std::size_t otherParticleSizeCount = otherParticleSystemPositionsArray.size();

			for (std::size_t particleIndex = 0; particleIndex < otherParticleSizeCount; ++particleIndex)
			{
				if ((currentParticlePosition - otherParticleSystemPositionsArray[particleIndex]).squaredNorm() < kernelLengthSquared)
				{
					currentNeighborhoodToFill.emplace_back(otherParticleSystemId, particleIndex);
				}
			}
		});
	}
}

void Storm::FluidParticleSystem::executePCISPH()
{
	// TODO
}

void Storm::FluidParticleSystem::initializeIteration()
{
	Storm::ParticleSystem::initializeIteration();

	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const Storm::Vector3 &gravity = configMgr.getGeneralSimulationData()._gravity;

	std::for_each(std::execution::par_unseq, std::begin(_accelerations), std::end(_accelerations), [&gravity](Storm::Vector3 &accel)
	{
		accel = gravity;
	});
}

void Storm::FluidParticleSystem::updatePosition(float deltaTimeInSec)
{
	std::for_each(std::execution::par_unseq, std::begin(_accelerations), std::end(_accelerations), [this, deltaTimeInSec](const Storm::Vector3 &currentAccel)
	{
		const std::size_t currentParticleIndex = this->getParticleIndex(_accelerations, currentAccel);

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
