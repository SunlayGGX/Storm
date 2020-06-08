#include "GraphicParticleData.h"



Storm::GraphicParticleData::GraphicParticleData(const Storm::Vector3 &position, const Storm::GraphicParticleData::ColorType &color) :
	_pos{ position.x(), position.y(), position.z(), 1.f },
	_color{ color }
{

}

