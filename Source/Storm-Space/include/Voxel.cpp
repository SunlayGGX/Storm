#include "Voxel.h"

#include "NeighborParticleReferral.h"


Storm::Voxel::Voxel()
{
	_particleReferralsData.reserve(64);
}

Storm::Voxel::Voxel(const Storm::Voxel &other) :
	_particleReferralsData{ other._particleReferralsData }
{

}

Storm::Voxel::~Voxel() = default;

void Storm::Voxel::clear()
{
	_particleReferralsData.clear();
}

void Storm::Voxel::addParticle(const std::size_t index, const unsigned int systemId)
{
	_particleReferralsData.emplace_back(index, systemId);
}

const std::vector<Storm::NeighborParticleReferral>& Storm::Voxel::getData() const
{
	return _particleReferralsData;
}
