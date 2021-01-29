#pragma once

#include "ParticleSystemContainer.h"


namespace Storm
{
	class ParticleCountInfo
	{
	public:
		ParticleCountInfo(const Storm::ParticleSystemContainer &pSystemContainer);

	public:
		std::size_t _totalParticleCount;
		std::size_t _fluidParticleCount;
		std::size_t _rigidbodiesParticleCount;
		std::size_t _staticRigidbodiesParticleCount;
		std::size_t _dynamicRigidbodiesParticleCount;
	};
}
