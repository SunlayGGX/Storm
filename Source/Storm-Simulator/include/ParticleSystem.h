#pragma once

#include "NeighborParticleInfo.h"
#include "ParticleSystemContainer.h"

namespace Storm
{
	class IBlower;

	class ParticleSystem
	{
	public:
		ParticleSystem(unsigned int particleSystemIndex, std::vector<Storm::Vector3> &&worldPositions);
		ParticleSystem(unsigned int particleSystemIndex, const std::size_t particleCount);
		virtual ~ParticleSystem() = default;

	private:
		void initParticlesCount(const std::size_t particleCount);
		void resizeParticlesCount(const std::size_t particleCount);

	public:
		const std::vector<Storm::Vector3>& getPositions() const noexcept;
		std::vector<Storm::Vector3>& getPositions() noexcept;
		const std::vector<Storm::Vector3>& getVelocity() const noexcept;
		std::vector<Storm::Vector3>& getVelocity() noexcept;
		const std::vector<Storm::Vector3>& getForces() const noexcept;
		std::vector<Storm::Vector3>& getForces() noexcept;

		const std::vector<Storm::Vector3>& getTemporaryPressureForces() const noexcept;
		std::vector<Storm::Vector3>& getTemporaryPressureForces() noexcept;
		const std::vector<Storm::Vector3>& getTemporaryViscosityForces() const noexcept;
		std::vector<Storm::Vector3>& getTemporaryViscosityForces() noexcept;

		const std::vector<Storm::ParticleNeighborhoodArray>& getNeighborhoodArrays() const noexcept;
		std::vector<Storm::ParticleNeighborhoodArray>& getNeighborhoodArrays() noexcept;

		std::size_t getParticleCount() const noexcept;

		unsigned int getId() const noexcept;

		virtual bool isFluids() const noexcept = 0;
		virtual bool isStatic() const noexcept = 0;
		virtual bool isWall() const noexcept = 0;

		bool isDirty() const noexcept;
		void setIsDirty(bool dirty);

		// Only to be used with replaying feature.
		virtual void setPositions(std::vector<Storm::Vector3> &&positions) = 0;
		virtual void setVelocity(std::vector<Storm::Vector3> &&velocities) = 0;
		virtual void setForces(std::vector<Storm::Vector3> &&forces) = 0;
		virtual void setTmpPressureForces(std::vector<Storm::Vector3> &&tmpPressureForces) = 0;
		virtual void setTmpViscosityForces(std::vector<Storm::Vector3> &&tmpViscoForces) = 0;
		virtual void setParticleSystemPosition(const Storm::Vector3 &pSystemPosition) = 0;
		virtual void setParticleSystemTotalForce(const Storm::Vector3 &pSystemTotalForce) = 0;

	public:
		void buildNeighborhood(const Storm::ParticleSystemContainer &allParticleSystems);

	protected:
		virtual void buildNeighborhoodOnParticleSystem(const Storm::ParticleSystem &otherParticleSystem, const float kernelLengthSquared) = 0;
		virtual void buildNeighborhoodOnParticleSystemUsingSpacePartition(const Storm::ParticleSystemContainer &allParticleSystems, const float kernelLengthSquared) = 0;

	public:
		virtual void updatePosition(float deltaTimeInSec, bool force) = 0;

	public:
		virtual void initializePreSimulation(const Storm::ParticleSystemContainer &allParticleSystems, const float kernelLength);

		virtual void onIterationStart();
		virtual void onSubIterationStart(const Storm::ParticleSystemContainer &allParticleSystems, const std::vector<std::unique_ptr<Storm::IBlower>> &blowers);

	public:
		virtual bool computeVelocityChange(float deltaTimeInSec, float highVelocityThresholdSquared) = 0;

	public:
		static float computeParticleDefaultVolume();

	public:
		static bool isElligibleNeighborParticle(const float kernelLengthSquared, const float normSquared);

	public:
		virtual void revertToCurrentTimestep(const std::vector<std::unique_ptr<Storm::IBlower>> &blowers) = 0;

	protected:
		std::vector<Storm::Vector3> _positions;
		std::vector<Storm::Vector3> _velocity;
		std::vector<Storm::Vector3> _force;

		// Tmp force value only valid if we're selecting a force, or if we are recording.
		std::vector<Storm::Vector3> _tmpPressureForce;
		std::vector<Storm::Vector3> _tmpViscosityForce;

		// This contains the neighborhood per particle.
		// Note : For static rigid body, it does not contain the static particles neighborhood because we use it only one time (when initializing the volume) and this is a huge lost of computation time !
		// So when we would need to have the static neighborhood for static rigid body, we should add another array separated from this one and compute it only one time 
		// => Static rigid bodies doesn't move, so static particles from static rigid bodies would always be the same, at the same position. Thus recomputing it each timestep takes a lot of time for nothing (for getting the same result as the time step before). Therefore, storing the result used from this static neighborhood, instead of recomputing it, is wiser.
		std::vector<Storm::ParticleNeighborhoodArray> _neighborhood;

		unsigned int _particleSystemIndex;
		bool _isDirty;

		float _maxVelocityNorm;

	public:
		mutable std::mutex _mutex;
	};
}
