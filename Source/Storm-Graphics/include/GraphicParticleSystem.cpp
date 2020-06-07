#include "GraphicParticleSystem.h"


Storm::GraphicParticleSystem::GraphicParticleSystem()
{

}

void Storm::GraphicParticleSystem::refreshParticleSystemData(unsigned int particleSystemId, std::vector<Storm::Vector3> &&particlePosition)
{
	_particleSystemPositions[particleSystemId] = std::move(particlePosition);
}
