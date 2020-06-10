#pragma once

#include "ParticleSystem.h"


namespace Storm
{
	class FluidParticleSystem : public Storm::ParticleSystem
	{
	public:
		using Storm::ParticleSystem::ParticleSystem;

	public:
		bool isFluids() const noexcept final override;

	public:
		void initializeIteration() final override;
		void updatePosition(float deltaTimeInSec) final override;
	};
}
