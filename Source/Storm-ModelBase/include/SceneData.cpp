#include "DataIncludes.h"


Storm::RigidBodySceneData::RigidBodySceneData() :
	_static{ true },
	_scale{ Storm::Vector3{ 1.f, 1.f, 1.f } }
{

}

Storm::SceneData::SceneData() :
	_gravity{ 0.f, -9.81f, 0.f }
{
	_rigidBodiesData.reserve(4);
}
