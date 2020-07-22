#pragma once

#include "ParticleSystem.h"


namespace Storm
{
	class FluidParticleSystem : public Storm::ParticleSystem
	{
	public:
		FluidParticleSystem(unsigned int particleSystemIndex, std::vector<Storm::Vector3> &&worldPositions);

	public:
		void initializeIteration() final override;

	public:
		bool isFluids() const noexcept final override;
		bool isStatic() const noexcept final override;
		bool isWall() const noexcept final override;

	public:
		// Accessible by dynamic casting.
		float getRestDensity() const noexcept;

	public:
		void buildNeighborhoodOnParticleSystem(const Storm::ParticleSystem &otherParticleSystem, const float kernelLengthSquared) final override;

	public:
		void updatePosition(float deltaTimeInSec) final override;

	private:
		std::vector<float> _masses;

		float _restDensity;
	};
}
