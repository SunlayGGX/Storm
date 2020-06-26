#pragma once


namespace Storm
{
	class ParticleSystem;

	// Just a structure to identify and retrieve the particle (neighborhood) from the data oriented architecture we took.
	// It has some data we had when we build the neighborhood to help us not recomping them again.
	struct NeighborParticleInfo
	{
	public:
		NeighborParticleInfo(const Storm::ParticleSystem* containingParticleSystem, std::size_t particleIndex, const Storm::Vector3 &positionDifferenceVector, float squaredNorm, bool isFluidP) :
			_containingParticleSystem{ containingParticleSystem },
			_particleIndex{ particleIndex },
			_positionDifferenceVector{ positionDifferenceVector },
			_vectToParticleSquaredNorm{ squaredNorm },
			_vectToParticleNorm{ std::sqrtf(squaredNorm) },
			_isFluidParticle{ isFluidP }
		{}

		~NeighborParticleInfo() = default;

	public:
		const Storm::ParticleSystem* _containingParticleSystem;
		std::size_t _particleIndex;
		const Storm::Vector3 _positionDifferenceVector; // currentP_position - neighborhoodP_Position
		float _vectToParticleSquaredNorm; // Norm squared of _positionDifferenceVector
		float _vectToParticleNorm; // the version non squared of _vectToParticleSquaredNorm
		bool _isFluidParticle;
	};
}
