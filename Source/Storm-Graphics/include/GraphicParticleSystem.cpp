#include "GraphicParticleSystem.h"


Storm::GraphicParticleSystem::GraphicParticleSystem()
{

}

void Storm::GraphicParticleSystem::refreshParticleSystemData(unsigned int particleSystemId, std::vector<DirectX::XMFLOAT3> &&particlePosition)
{
	_particleSystemPositions[particleSystemId] = std::move(particlePosition);
}
