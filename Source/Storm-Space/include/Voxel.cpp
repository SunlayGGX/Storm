#include "Voxel.h"
#include "PositionVoxel.h"

#include "NeighborParticleReferral.h"


Storm::Voxel::Voxel()
{
	_particleReferralsData.reserve(64);
}

Storm::Voxel::Voxel(Storm::Voxel &&other) = default;
Storm::Voxel::Voxel(const Storm::Voxel &other) = default;
Storm::Voxel::~Voxel() = default;

void Storm::Voxel::clear()
{
	_particleReferralsData.clear();
}

void Storm::Voxel::addParticle(const std::size_t index, const unsigned int systemId)
{
	_particleReferralsData.emplace_back(index, systemId);
}

Storm::PositionVoxel::PositionVoxel() = default;
Storm::PositionVoxel::PositionVoxel(Storm::PositionVoxel &&other) = default;
Storm::PositionVoxel::PositionVoxel(const Storm::PositionVoxel &other) = default;
Storm::PositionVoxel::~PositionVoxel() = default;

void Storm::PositionVoxel::clear()
{
	_positionData.clear();
}

void Storm::PositionVoxel::addData(const Storm::Vector3 &pos)
{
	_positionData.emplace_back(pos);
}
