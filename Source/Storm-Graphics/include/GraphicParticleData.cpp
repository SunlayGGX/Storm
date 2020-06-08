#include "GraphicParticleData.h"



Storm::GraphicParticleData::GraphicParticleData(const Storm::Vector3 &position, const float particleSize) :
	_pos{ position.x(), position.y(), position.z(), 1.f },
	_pointSize{ particleSize }
{

}

