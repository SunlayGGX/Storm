#include "DataIncludes.h"

#include "CollisionType.h"


Storm::GeneralSimulationData::GeneralSimulationData() :
	_gravity{ 0.f, -9.81f, 0.f },
	_particleRadius{ 0.05f },
	_kernelCoefficient{ 4.f }
{

}

Storm::RigidBodySceneData::RigidBodySceneData() :
	_static{ true },
	_scale{ Storm::Vector3{ 1.f, 1.f, 1.f } },
	_collisionShape{ Storm::CollisionType::None }
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
	_generalSimulationData{ std::make_unique<Storm::GeneralSimulationData>() },
	_graphicData{ std::make_unique<Storm::GraphicData>() }
{
	_rigidBodiesData.reserve(4);
}
