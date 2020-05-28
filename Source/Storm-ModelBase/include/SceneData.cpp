#include "DataIncludes.h"


Storm::RigidBodySceneData::RigidBodySceneData() :
	_static{ true },
	_scale{ Storm::Vector3{ 1.f, 1.f, 1.f } }
{

}

Storm::GraphicData::GraphicData() :
	_cameraPosition{ 0.f, 0.f, -10.f },
	_cameraLookAt{ 0.f, 0.f, 0.f },
	_zNear{ 0.01f },
	_zFar{ 20.f }
{

}

Storm::SceneData::SceneData() :
	_gravity{ 0.f, -9.81f, 0.f },
	_graphicData{ std::make_unique<Storm::GraphicData>() }
{
	_rigidBodiesData.reserve(4);
}
