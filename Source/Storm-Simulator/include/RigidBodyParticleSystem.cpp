#include "RigidBodyParticleSystem.h"

#include "SingletonHolder.h"
#include "IPhysicsManager.h"


bool Storm::RigidBodyParticleSystem::isFluids() const noexcept
{
	return false;
}

void Storm::RigidBodyParticleSystem::updatePosition(float deltaTimeInSec)
{
	Storm::Vector3 currentPosition;
	Storm::Vector3 currentRotation;
	Storm::SingletonHolder::instance().getSingleton<Storm::IPhysicsManager>().getMeshTransform(_particleSystemIndex, currentPosition, currentRotation);

	_isDirty = currentPosition != _cachedTrackedRbPosition || currentRotation != _cachedTrackedRbRotation;

}
