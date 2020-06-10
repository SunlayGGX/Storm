#include "RigidBodyParticleSystem.h"


bool Storm::RigidBodyParticleSystem::isFluids() const noexcept
{
	return false;
}

void Storm::RigidBodyParticleSystem::updatePosition(float deltaTimeInSec)
{
	// TODO : The particle shouldn't go everywhere since it is a "rigid" body...
	// The displacement was computed by Physics so use the displacement to compute the same move vector to be applied to all rb particles.
}
