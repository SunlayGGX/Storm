#pragma once


namespace Storm
{
	class ParticleSystem;

	// Just a structure to identify and retrieve the particle from the data oriented architecture we took.
	struct ParticleIdentifier
	{
	public:
		ParticleIdentifier(const Storm::ParticleSystem* containingParticleSystem, std::size_t particleIndex, float squaredNorm) :
			_containingParticleSystem{ containingParticleSystem },
			_particleIndex{ particleIndex },
			_vectToParticleSquaredNorm{ squaredNorm }
		{}

		~ParticleIdentifier() = default;

	public:
		const Storm::ParticleSystem* _containingParticleSystem;
		std::size_t _particleIndex;
		float _vectToParticleSquaredNorm;
	};
}
