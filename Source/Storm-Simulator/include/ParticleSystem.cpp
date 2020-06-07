#include "ParticleSystem.h"



Storm::ParticleSystem::ParticleSystem(std::vector<Storm::Vector3> &&worldPositions) :
	_positions{ std::move(worldPositions) }
{

}

const std::vector<Storm::Vector3>& Storm::ParticleSystem::getPositions() const
{
	return _positions;
}
