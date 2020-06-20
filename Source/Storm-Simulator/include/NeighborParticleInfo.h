#pragma once


namespace Storm
{
	class ParticleSystem;

	// Just a structure to identify and retrieve the particle (neighborhood) from the data oriented architecture we took.
	// It has some data we had when we build the neighborhood to help us not recomping them again.
	struct NeighborParticleInfo
	{
	public:
		NeighborParticleInfo(const Storm::ParticleSystem* containingParticleSystem, std::size_t particleIndex, float squaredNorm) :
			_containingParticleSystem{ containingParticleSystem },
			_particleIndex{ particleIndex },
			_vectToParticleSquaredNorm{ squaredNorm }
		{}

		~NeighborParticleInfo() = default;

	public:
		const Storm::ParticleSystem* _containingParticleSystem;
		std::size_t _particleIndex;
		float _vectToParticleSquaredNorm;
	};
}
