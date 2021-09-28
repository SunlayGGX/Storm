#pragma once

#include "ParticleSystem.h"


namespace Storm
{
	class RigidBodyParticleSystem : public Storm::ParticleSystem
	{
	public:
		RigidBodyParticleSystem(unsigned int particleSystemIndex, std::vector<Storm::Vector3> &&worldPositions);
		RigidBodyParticleSystem(unsigned int particleSystemIndex, const std::size_t particleCount);

	private:
		void loadRigidbodyConfig();

	public:
		void initializePreSimulation(const Storm::ParticleSystemContainer &allParticleSystems, const float kernelLength) final override;

		void onIterationStart() final override;
		void onSubIterationStart(const Storm::ParticleSystemContainer &allParticleSystems, const std::vector<std::unique_ptr<Storm::IBlower>> &blowers) final override;

	public:
		bool isFluids() const noexcept final override;
		bool isStatic() const noexcept final override;
		bool isWall() const noexcept final override;

		void setPositions(std::vector<Storm::Vector3> &&positions) final override;
		void setVelocity(std::vector<Storm::Vector3> &&velocities) final override;
		void setForces(std::vector<Storm::Vector3> &&forces) final override;
		void setDensities(std::vector<float> &&densities) final override;
		void setPressures(std::vector<float> &&pressures) final override;
		void setVolumes(std::vector<float> &&volumes) final override;
		void setMasses(std::vector<float> &&masses) final override;
		void setNormals(std::vector<Storm::Vector3> &&normals) final override;
		void setTmpPressureForces(std::vector<Storm::Vector3> &&tmpPressureForces) final override;
		void setTmpViscosityForces(std::vector<Storm::Vector3> &&tmpViscoForces) final override;
		void setTmpDragForces(std::vector<Storm::Vector3> &&tmpDragForces) final override;
		void setTmpBernoulliDynamicPressureForces(std::vector<Storm::Vector3> &&tmpDynamicQForces) final override;
		void setTmpNoStickForces(std::vector<Storm::Vector3> &&tmpNoStick) final override;
		void setTmpPressureIntermediaryForces(std::vector<Storm::Vector3> &&tmpPressuresIntermediaryForces) final override;
		void setParticleSystemPosition(const Storm::Vector3 &pSystemPosition) final override;
		void setParticleSystemTotalForce(const Storm::Vector3 &pSystemTotalForce) final override;

	public:
		const std::vector<float>& getVolumes() const noexcept;
		std::vector<float>& getVolumes() noexcept;

		const std::vector<Storm::Vector3>& getNormals() const noexcept;
		std::vector<Storm::Vector3>& getNormals() noexcept;

		float getViscosity() const noexcept;
		float getNoStickCoefficient() const noexcept;
		float getDragCoefficient() const noexcept;

		const Storm::Vector3& getRbPosition() const noexcept;
		const Storm::Vector3& getRbTotalForce() const noexcept;

	private:
		void buildNeighborhoodOnParticleSystemUsingSpacePartition(const Storm::ParticleSystemContainer &allParticleSystems, const float kernelLength) final override;

	public:
		bool computeVelocityChange(float deltaTimeInSec, float highVelocityThresholdSquared) final override;

	public:
		void updatePosition(float deltaTimeInSec, bool force) final override;

	public:
		void revertToCurrentTimestep(const std::vector<std::unique_ptr<Storm::IBlower>> &blowers) final override;

	private:
		// Volumes got from the computation of static rigid bodies neighbor for a rigid body (So useful only for Rigid bodies) !
		std::vector<float> _staticVolumesInitValue;

		std::vector<float> _volumes;

		std::vector<Storm::Vector3> _normals;

		float _viscosity;
		float _noStickCoefficient;
		float _dragCoefficient;

		// Those are particle global position and rotation that serves to update particles using the rigid body position and rotation owned by the physics engine.
		// Beware because they are made to track the changes of position and rotation, it means that they are not forcefully up to date.
		// They will be after calling updatePosition, but between the moment the physics engine computed the changes and the next updatePosition,
		// they will be equal to the position of the last frame.
		Storm::Vector3 _cachedTrackedRbPosition;
		Storm::Quaternion _cachedTrackedRbRotationQuat;

		// Account for PhysX
		Storm::Vector3 _rbTotalForce;

		bool _volumeFixed;

		bool _isStatic;
		bool _isWall;
		bool _velocityDirtyInternal;
	};
}
