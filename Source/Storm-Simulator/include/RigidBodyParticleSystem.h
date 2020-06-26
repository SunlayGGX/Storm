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

	public:
		const std::vector<float>& getPredictedDensities() const final override;
		std::vector<float>& getPredictedDensities() final override;
		const std::vector<Storm::Vector3>& getPredictedPositions() const final override;
		std::vector<Storm::Vector3>& getPredictedPositions() final override;
		const std::vector<Storm::Vector3>& getPredictedPressureForces() const final override;
		std::vector<Storm::Vector3>& getPredictedPressureForces() final override;

	private:
		void buildNeighborhoodOnParticleSystem(const Storm::ParticleSystem &otherParticleSystem, const float kernelLengthSquared) final override;

	public:
		void updatePosition(float deltaTimeInSec) final override;

	public:
		void postApplySPH() final override;

	private:
		// Those are particle global position and rotation that serves to update particles using the rigid body position and rotation owned by the physics engine.
		// Beware because they are made to track the changes of position and rotation, it means that they are not forcefully up to date.
		// They will be after calling updatePosition, but between the moment the physics engine computed the changes and the next updatePosition,
		// they will be equal to the position of the last frame.
		Storm::Vector3 _cachedTrackedRbPosition;
		Storm::Quaternion _cachedTrackedRbRotationQuat;
	};
}
