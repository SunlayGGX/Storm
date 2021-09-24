#pragma once


namespace Storm
{
	class ParticleSystem;

	// Just a structure to identify and retrieve the particle (neighborhood) from the data oriented architecture we took.
	// It has some data we had when we build the neighborhood to help us not recomputing them again.
	struct NeighborParticleInfo
	{
	public:
		NeighborParticleInfo(Storm::ParticleSystem*const containingParticleSystem, std::size_t particleIndex, const Storm::Vector3 &positionDifferenceVector, float squaredNorm) :
			_containingParticleSystem{ containingParticleSystem },
			_particleIndex{ particleIndex },
			_xij{ positionDifferenceVector },
			_xijSquaredNorm{ squaredNorm },
			_xijNorm{ std::sqrtf(squaredNorm) }
		{}

		NeighborParticleInfo(Storm::ParticleSystem*const containingParticleSystem, std::size_t particleIndex, const float xDiff, const float yDiff, const float zDiff, const float squaredNorm) :
			_containingParticleSystem{ containingParticleSystem },
			_particleIndex{ particleIndex },
			_xij{ xDiff, yDiff, zDiff },
			_xijSquaredNorm{ squaredNorm },
			_xijNorm{ std::sqrtf(squaredNorm) }
		{}

		~NeighborParticleInfo() = default;

	public:
		Storm::ParticleSystem*const _containingParticleSystem;
		const std::size_t _particleIndex;
		const Storm::Vector3 _xij; // currentP_position - neighborhoodP_Position
		const float _xijSquaredNorm; // Norm squared of _positionDifferenceVector
		const float _xijNorm; // the version non squared of _vectToParticleSquaredNorm

		// Making a cache of those value made an optimization of around 30% of the computational time
		// (tested on 5 runs with and without those caches with the same settings and IISPH :
		// with : average speed : 1s real time to simulate 1.396338s virtual simulation time
		// without : average speed : 1s real time to simulate 1.079538s virtual simulation time
		float _Wij;
		Storm::Vector3 _gradWij;
	};

	using ParticleNeighborhoodArray = std::vector<Storm::NeighborParticleInfo>;
}
