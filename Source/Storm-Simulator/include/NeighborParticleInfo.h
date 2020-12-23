#pragma once


namespace Storm
{
	class ParticleSystem;

	// Just a structure to identify and retrieve the particle (neighborhood) from the data oriented architecture we took.
	// It has some data we had when we build the neighborhood to help us not recomping them again.
	struct NeighborParticleInfo
	{
	public:
		NeighborParticleInfo(Storm::ParticleSystem*const containingParticleSystem, std::size_t particleIndex, const Storm::Vector3 &positionDifferenceVector, float squaredNorm, bool isFluidP) :
			_containingParticleSystem{ containingParticleSystem },
			_particleIndex{ particleIndex },
			_positionDifferenceVector{ positionDifferenceVector },
			_vectToParticleSquaredNorm{ squaredNorm },
			_vectToParticleNorm{ std::sqrtf(squaredNorm) },
			_isFluidParticle{ isFluidP }
		{}

		NeighborParticleInfo(Storm::ParticleSystem*const containingParticleSystem, std::size_t particleIndex, const float xDiff, const float yDiff, const float zDiff, const float squaredNorm, const bool isFluidP) :
			_containingParticleSystem{ containingParticleSystem },
			_particleIndex{ particleIndex },
			_positionDifferenceVector{ xDiff, yDiff, zDiff },
			_vectToParticleSquaredNorm{ squaredNorm },
			_vectToParticleNorm{ std::sqrtf(squaredNorm) },
			_isFluidParticle{ isFluidP }
		{}

		~NeighborParticleInfo() = default;

	public:
		Storm::ParticleSystem*const _containingParticleSystem;
		const std::size_t _particleIndex;
		const Storm::Vector3 _positionDifferenceVector; // currentP_position - neighborhoodP_Position
		const float _vectToParticleSquaredNorm; // Norm squared of _positionDifferenceVector
		const float _vectToParticleNorm; // the version non squared of _vectToParticleSquaredNorm
		const bool _isFluidParticle;

		float _Wij;
		Storm::Vector3 _gradWij;
	};

	using ParticleNeighborhoodArray = std::vector<Storm::NeighborParticleInfo>;
}
