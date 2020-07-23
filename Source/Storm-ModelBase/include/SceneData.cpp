#include "DataIncludes.h"

#include "CollisionType.h"
#include "SimulationMode.h"
#include "KernelMode.h"


Storm::GeneralSimulationData::GeneralSimulationData() :
	_gravity{ 0.f, -9.81f, 0.f },
	_particleRadius{ 0.05f },
	_kernelCoefficient{ 4.f },
	_cflCoeff{ 0.4f },
	_startPaused{ false },
	_physicsTimeInSeconds{ -1.f },
	_expectedFps{ -1.f },
	_simulationMode{ Storm::SimulationMode::None },
	_maxPredictIteration{ 150 },
	_maxDensityError{ 0.01f },
	_kernelMode{ Storm::KernelMode::CubicSpline },
	_maxCFLTime{ 0.5f }
{

}

Storm::RigidBodySceneData::RigidBodySceneData() :
	_static{ true },
	_isWall{ false },
	_translation{ 0.f, 0.f, 0.f },
	_rotation{ 0.f, 0.f, 0.f },
	_scale{ 1.f, 1.f, 1.f },
	_collisionShape{ Storm::CollisionType::None },
	_staticFrictionCoefficient{ 0.1f },
	_dynamicFrictionCoefficient{ 0.f },
	_restitutionCoefficient{ 0.1f },
	_mass{ -1.f },
	_viscosity{ 0.f }
{

}

Storm::GraphicData::GraphicData() :
	_cameraPosition{ 0.f, 0.f, -10.f },
	_cameraLookAt{ 0.f, 0.f, 0.f },
	_zNear{ 0.01f },
	_zFar{ 20.f },
	_grid{ 10.f, 0.f, 10.f },
	_displaySolidAsParticles{ false },
	_valueForMinColor{ 0.01f },
	_valueForMaxColor{ 100.f }
{

}

Storm::FluidBlockData::FluidBlockData() :
	_firstPoint{ Vector3::Zero() },
	_secondPoint{ Vector3::Zero() }
{

}

Storm::FluidData::FluidData() :
	_density{ 1.2754f }, // Dry air density at 0 °C degrees and normal ATM pressure. https://en.wikipedia.org/wiki/Density_of_air.
	_dynamicViscosity{ 0.00001715f }, // Dry air dynamic viscosity at 0 °C degrees and normal ATM pressure. https://www.engineeringtoolbox.com/air-absolute-kinematic-viscosity-d_601.html.
	_soundSpeed{ 331.4f }, // Sound speed in air at 0 °C degrees and normal ATM pressure. https://www.engineeringtoolbox.com/air-speed-sound-d_603.html,
	_kPressureStiffnessCoeff{ 50.f },
	_kPressureExponentCoeff{ 7.f },
	_cinematicViscosity{ 0.f } // Computed automatically once final _dynamicViscosity value will be determined.
{

}

Storm::SceneData::SceneData() :
	_generalSimulationData{ std::make_unique<Storm::GeneralSimulationData>() },
	_graphicData{ std::make_unique<Storm::GraphicData>() },
	_fluidData{ std::make_unique<Storm::FluidData>() }
{
	_rigidBodiesData.reserve(4);
}
