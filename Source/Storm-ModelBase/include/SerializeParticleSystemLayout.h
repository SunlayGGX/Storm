#pragma once


namespace Storm
{
	struct SerializeParticleSystemLayout
	{
		uint32_t _particleSystemId;
		uint64_t _particlesCount;
		bool _isFluid;
		bool _isStatic;
	};
}
