#pragma once


namespace Storm
{
	struct IterationParameter
	{
	public:
		Storm::ParticleSystemContainer*const _particleSystems;
		const float _kernelLength;
		const float _kernelLengthSquared;
		const float _deltaTime;
	};
}
