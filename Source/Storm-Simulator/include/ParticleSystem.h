#pragma once

#include "NeighborParticleInfo.h"

namespace Storm
{
	class ParticleSystem
	{
	public:
		using ParticleNeighborhoodArray = std::vector<Storm::NeighborParticleInfo>;

	public:
		ParticleSystem(unsigned int particleSystemIndex, std::vector<Storm::Vector3> &&worldPositions);
		virtual ~ParticleSystem() = default;

	public:
		const std::vector<Storm::Vector3>& getPositions() const noexcept;
		std::vector<Storm::Vector3>& getPositions() noexcept;
		const std::vector<Storm::Vector3>& getVelocity() const noexcept;
		std::vector<Storm::Vector3>& getVelocity() noexcept;
		const std::vector<Storm::Vector3>& getForces() const noexcept;
		std::vector<Storm::Vector3>& getForces() noexcept;

		const std::vector<std::vector<Storm::NeighborParticleInfo>> &getNeighborhoodArrays() const noexcept;
		std::vector<std::vector<Storm::NeighborParticleInfo>> &getNeighborhoodArrays() noexcept;

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
		static float computeParticleDefaultVolume();

	protected:
		static bool isElligibleNeighborParticle(const float kernelLengthSquared, const float normSquared);

	protected:
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
