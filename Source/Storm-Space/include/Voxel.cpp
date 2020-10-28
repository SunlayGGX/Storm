#include "Voxel.h"
#include "PositionVoxel.h"

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

Storm::PositionVoxel::PositionVoxel()
{

}

Storm::PositionVoxel::PositionVoxel(const Storm::PositionVoxel &other) :
	_positionData{ other._positionData }
{

}

Storm::PositionVoxel::~PositionVoxel() = default;

void Storm::PositionVoxel::clear()
{
	_positionData.clear();
}

void Storm::PositionVoxel::addData(const Storm::Vector3 &pos)
{
	_positionData.emplace_back(pos);
}
