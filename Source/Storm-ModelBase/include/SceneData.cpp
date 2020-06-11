#include "DataIncludes.h"

#include "CollisionType.h"


Storm::GeneralSimulationData::GeneralSimulationData() :
	_gravity{ 0.f, -9.81f, 0.f },
	_particleRadius{ 0.05f },
	_kernelCoefficient{ 4.f },
	_startPaused{ false },
	_physicsTimeInSeconds{ -1.f }
{

}

Storm::RigidBodySceneData::RigidBodySceneData() :
	_static{ true },
	_translation{ 0.f, 0.f, 0.f },
	_rotation{ 0.f, 0.f, 0.f },
	_scale{ 1.f, 1.f, 1.f },
	_collisionShape{ Storm::CollisionType::None },
	_staticFrictionCoefficient{ 0.1f },
	_dynamicFrictionCoefficient{ 0.f },
	_restitutionCoefficient{ 0.1f }
{

}

Storm::GraphicData::GraphicData() :
	_cameraPosition{ 0.f, 0.f, -10.f },
	_cameraLookAt{ 0.f, 0.f, 0.f },
	_zNear{ 0.01f },
	_zFar{ 20.f },
	_grid{ 10.f, 0.f, 10.f },
	_displaySolidAsParticles{ false }
{

}

Storm::FluidBlockData::FluidBlockData() :
	_firstPoint{ Vector3::Zero() },
	_secondPoint{ Vector3::Zero() }
{

}

Storm::FluidData::FluidData()
{

}

Storm::SceneData::SceneData() :
	_generalSimulationData{ std::make_unique<Storm::GeneralSimulationData>() },
	_graphicData{ std::make_unique<Storm::GraphicData>() },
	_fluidData{ std::make_unique<Storm::FluidData>() }
{
	_rigidBodiesData.reserve(4);
}
