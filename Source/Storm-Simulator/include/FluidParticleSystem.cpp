#include "FluidParticleSystem.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "GeneralSimulationData.h"


bool Storm::FluidParticleSystem::isFluids() const noexcept
{
	return true;
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
		const std::size_t currentParticleIndex = &currentAccel - &_accelerations[0];

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
