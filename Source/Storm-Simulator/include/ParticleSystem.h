#pragma once

#include "ParticleIdentifier.h"

namespace Storm
{
	class ParticleSystem
	{
	public:
		ParticleSystem(unsigned int particleSystemIndex, std::vector<Storm::Vector3> &&worldPositions, float particleMass);
		virtual ~ParticleSystem() = default;

	public:
		const std::vector<float>& getDensities() const noexcept;
		const std::vector<Storm::Vector3>& getPositions() const noexcept;
		const std::vector<Storm::Vector3>& getVelocity() const noexcept;
		const std::vector<Storm::Vector3>& getAccelerations() const noexcept;
		const std::vector<Storm::Vector3>& getForces() const noexcept;

		unsigned int getId() const noexcept;

		virtual bool isFluids() const noexcept = 0;

		bool isDirty() const noexcept;

		void buildNeighborhood(const std::map<unsigned int, std::unique_ptr<Storm::ParticleSystem>> &allParticleSystems);

	protected:
		virtual void buildNeighborhoodOnParticleSystem(const Storm::ParticleSystem &otherParticleSystem, const float kernelLengthSquared) = 0;

	public:
		void executeSPH(const std::map<unsigned int, std::unique_ptr<Storm::ParticleSystem>> &allParticleSystems);

	protected:
		virtual void executePCISPH() = 0;

	public:
		virtual void postApplySPH();

	public:
		virtual void initializeIteration();
		virtual void updatePosition(float deltaTimeInSec) = 0;

	public:
		static float computeParticleDefaultVolume();

	public:
		template<class Type>
		std::size_t getParticleIndex(const std::vector<std::remove_cv_t<Type>> &dataArray, Type &particleData) const
		{
			return &particleData - &dataArray[0];
		}


	protected:
		std::vector<float> _densities;
		std::vector<Storm::Vector3> _positions;
		std::vector<Storm::Vector3> _velocity;
		std::vector<Storm::Vector3> _accelerations;
		std::vector<Storm::Vector3> _force;

		std::vector<std::vector<Storm::ParticleIdentifier>> _neighborhood;

		float _massPerParticle;

		unsigned int _particleSystemIndex;
		std::atomic<bool> _isDirty;
	};
}
