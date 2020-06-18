#pragma once

#include "ParticleSystem.h"


namespace Storm
{
	class FluidParticleSystem : public Storm::ParticleSystem
	{
	public:
		FluidParticleSystem(unsigned int particleSystemIndex, std::vector<Storm::Vector3> &&worldPositions);

	public:
		bool isFluids() const noexcept final override;

	public:
		void buildNeighborhoodOnParticleSystem(const Storm::ParticleSystem &otherParticleSystem, const float kernelLengthSquared) final override;

	private:
		void executePCISPH() final override;

	public:
		void initializeIteration() final override;
		void updatePosition(float deltaTimeInSec) final override;
	};
}
