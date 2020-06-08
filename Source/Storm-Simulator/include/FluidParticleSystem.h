#pragma once

#include "ParticleSystem.h"


namespace Storm
{
	class FluidParticleSystem : public Storm::ParticleSystem
	{
	public:
		using Storm::ParticleSystem::ParticleSystem;

	public:
		virtual bool isFluids() const noexcept override;
	};
}
