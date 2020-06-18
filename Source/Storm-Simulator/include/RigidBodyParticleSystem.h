#pragma once

#include "ParticleSystem.h"


namespace Storm
{
	class RigidBodyParticleSystem : public Storm::ParticleSystem
	{
	public:
		RigidBodyParticleSystem(unsigned int particleSystemIndex, std::vector<Storm::Vector3> &&worldPositions);

	public:
		bool isFluids() const noexcept final override;

	private:
		void buildNeighborhoodOnParticleSystem(const Storm::ParticleSystem &otherParticleSystem, const float kernelLengthSquared) final override;

	private:
		void executePCISPH() final override;

	public:
		void updatePosition(float deltaTimeInSec) final override;

	private:
		// Those are particle global position and rotation that serves to update particles using the rigid body position and rotation owned by the physics engine.
		// Beware because they are made to track the changes of position and rotation, it means that they are not forcefully up to date.
		// They will be after calling updatePosition, but between the moment the physics engine computed the changes and the next updatePosition,
		// they will be equal to the position of the last frame.
		Storm::Vector3 _cachedTrackedRbPosition;
		Storm::Quaternion _cachedTrackedRbRotationQuat;
	};
}
