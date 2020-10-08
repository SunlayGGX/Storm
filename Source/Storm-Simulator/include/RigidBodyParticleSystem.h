#pragma once

#include "ParticleSystem.h"


namespace Storm
{
	class RigidBodyParticleSystem : public Storm::ParticleSystem
	{
	public:
		RigidBodyParticleSystem(unsigned int particleSystemIndex, std::vector<Storm::Vector3> &&worldPositions);
		RigidBodyParticleSystem(unsigned int particleSystemIndex, const std::size_t particleCount);

	public:
		void initializePreSimulation(const std::map<unsigned int, std::unique_ptr<Storm::ParticleSystem>> &allParticleSystems, const float kernelLength) final override;
		void initializeIteration(const std::map<unsigned int, std::unique_ptr<Storm::ParticleSystem>> &allParticleSystems, const std::vector<std::unique_ptr<Storm::IBlower>> &blowers, const bool shouldRegisterTemporaryForce) final override;

	public:
		bool isFluids() const noexcept final override;
		bool isStatic() const noexcept final override;
		bool isWall() const noexcept final override;

		void setPositions(std::vector<Storm::Vector3> &&positions) final override;
		void setVelocity(std::vector<Storm::Vector3> &&velocities) final override;
		void setForces(std::vector<Storm::Vector3> &&forces) final override;
		void setTmpPressureForces(std::vector<Storm::Vector3> &&tmpPressureForces) final override;
		void setTmpViscosityForces(std::vector<Storm::Vector3> &&tmpViscoForces) final override;

	public:
		const std::vector<float>& getVolumes() const noexcept;
		std::vector<float>& getVolumes() noexcept;

		float getViscosity() const noexcept;

	private:
		void buildNeighborhoodOnParticleSystem(const Storm::ParticleSystem &otherParticleSystem, const float kernelLengthSquared) final override;
		void buildNeighborhoodOnParticleSystemUsingSpacePartition(const std::map<unsigned int, std::unique_ptr<Storm::ParticleSystem>> &allParticleSystems, const float kernelLengthSquared) final override;

	public:
		bool computeVelocityChange(float deltaTimeInSec, float highVelocityThresholdSquared) final override;

	public:
		void updatePosition(float deltaTimeInSec, bool force) final override;
		void postApplySPH(float deltaTimeInSec) final override;

	public:
		void revertToCurrentTimestep(const std::vector<std::unique_ptr<Storm::IBlower>> &blowers, const bool shouldRegisterTemporaryForce) final override;

	private:
		// Volumes got from the computation of static rigid bodies neighbor for a rigid body (So useful only for Rigid bodies) !
		std::vector<float> _staticVolumesInitValue;

		std::vector<float> _volumes;

		float _viscosity;

		// Those are particle global position and rotation that serves to update particles using the rigid body position and rotation owned by the physics engine.
		// Beware because they are made to track the changes of position and rotation, it means that they are not forcefully up to date.
		// They will be after calling updatePosition, but between the moment the physics engine computed the changes and the next updatePosition,
		// they will be equal to the position of the last frame.
		Storm::Vector3 _cachedTrackedRbPosition;
		Storm::Quaternion _cachedTrackedRbRotationQuat;

		bool _isStatic;
		bool _isWall;
	};
}
