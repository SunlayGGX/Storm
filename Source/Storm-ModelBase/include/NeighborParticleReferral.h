#pragma once


namespace Storm
{
	struct NeighborParticleReferral
	{
	public:
		NeighborParticleReferral(std::size_t particleIndex, unsigned int systemId) :
			_particleIndex{ particleIndex },
			_systemId{ systemId }
		{}

	public:
		std::size_t _particleIndex;
		unsigned int _systemId;
	};
}
