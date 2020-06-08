#pragma once

#include "ParticleSystem.h"


namespace Storm
{
	class RigidBodyParticleSystem : public Storm::ParticleSystem
	{
	public:
		using Storm::ParticleSystem::ParticleSystem;

	public:
		virtual bool isFluids() const noexcept override;
	};
}
