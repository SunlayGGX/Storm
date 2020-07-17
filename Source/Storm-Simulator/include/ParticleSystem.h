#pragma once

#include "NeighborParticleInfo.h"

namespace Storm
{
	class ParticleSystem
	{
	public:
		using ParticleNeighborhoodArray = std::vector<Storm::NeighborParticleInfo>;

	public:
		ParticleSystem(unsigned int particleSystemIndex, std::vector<Storm::Vector3> &&worldPositions, float particleMass);
		virtual ~ParticleSystem() = default;

	public:
		const std::vector<float>& getMasses() const noexcept;
		std::vector<float>& getMasses() noexcept;
		const std::vector<float>& getDensities() const noexcept;
		std::vector<float>& getDensities() noexcept;
		const std::vector<Storm::Vector3>& getPositions() const noexcept;
		std::vector<Storm::Vector3>& getPositions() noexcept;
		const std::vector<Storm::Vector3>& getVelocity() const noexcept;
		std::vector<Storm::Vector3>& getVelocity() noexcept;
		const std::vector<Storm::Vector3>& getForces() const noexcept;
		std::vector<Storm::Vector3>& getForces() noexcept;

		const std::vector<std::vector<Storm::NeighborParticleInfo>> &getNeighborhoodArrays() const noexcept;
		std::vector<std::vector<Storm::NeighborParticleInfo>> &getNeighborhoodArrays() noexcept;

		virtual const std::vector<float>& getPressures() const = 0;
		virtual std::vector<float>& getPressures() = 0;
		virtual const std::vector<float>& getVolumes() const = 0;
		virtual std::vector<float>& getVolumes() = 0;
		virtual float getParticleVolume() const = 0;
		virtual float getRestDensity() const = 0;

		// "Predictive" SPH
		virtual const std::vector<float>& getPredictedDensities() const = 0;
		virtual std::vector<float>& getPredictedDensities() = 0;
		virtual const std::vector<Storm::Vector3>& getPredictedPositions() const = 0;
		virtual std::vector<Storm::Vector3>& getPredictedPositions() = 0;
		virtual const std::vector<Storm::Vector3>& getPredictedPressureForces() const = 0;
		virtual std::vector<Storm::Vector3>& getPredictedPressureForces() = 0;

		unsigned int getId() const noexcept;

		virtual bool isFluids() const noexcept = 0;
		virtual bool isStatic() const noexcept = 0;
		virtual bool isWall() const noexcept = 0;

		bool isDirty() const noexcept;

		void buildNeighborhood(const std::map<unsigned int, std::unique_ptr<Storm::ParticleSystem>> &allParticleSystems);

	protected:
		virtual void buildNeighborhoodOnParticleSystem(const Storm::ParticleSystem &otherParticleSystem, const float kernelLengthSquared) = 0;

	public:
		virtual void postApplySPH();

	public:
		virtual void initializeIteration();
		virtual void updatePosition(float deltaTimeInSec) = 0;

	public:
		// For predicted method (no check of the simulation method will be done)
		virtual void flushPressureToTotalForce() = 0;

	public:
		static float computeParticleDefaultVolume();

	protected:
		static bool isElligibleNeighborParticle(const float kernelLengthSquared, const float normSquared);

	protected:
		std::vector<float> _masses;
		std::vector<float> _densities;

		std::vector<Storm::Vector3> _positions;
		std::vector<Storm::Vector3> _velocity;
		std::vector<Storm::Vector3> _force;

		std::vector<ParticleNeighborhoodArray> _neighborhood;

		unsigned int _particleSystemIndex;
		std::atomic<bool> _isDirty;

	public:
		mutable std::mutex _mutex;
	};
}
