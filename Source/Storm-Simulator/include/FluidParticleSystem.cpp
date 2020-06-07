#include "FluidParticleSystem.h"



Storm::FluidParticleSystem::FluidParticleSystem(std::vector<Storm::Vector3> &&worldPositions) :
	_positions{ std::move(worldPositions) }
{

}

const std::vector<Storm::Vector3>& Storm::FluidParticleSystem::getPositions() const
{
	return _positions;
}
