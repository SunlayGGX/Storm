#pragma once


namespace Storm
{
	// Just a structure to identify and retrieve the particle from the data oriented architecture we took.
	struct ParticleIdentifier
	{
	public:
		ParticleIdentifier(unsigned int systemId, std::size_t particleIndex, float squaredNorm) :
			_particleSystemId{ systemId },
			_particleIndex{ particleIndex },
			_vectToParticleSquaredNorm{ squaredNorm }
		{}

		~ParticleIdentifier() = default;

	public:
		unsigned int _particleSystemId;
		std::size_t _particleIndex;
		float _vectToParticleSquaredNorm;
	};
}