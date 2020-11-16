#include "SceneData.h"
#include "GeneralSimulationData.h"
#include "RigidBodySceneData.h"
#include "GraphicData.h"
#include "FluidData.h"
#include "BlowerData.h"
#include "ConstraintData.h"
#include "RecordConfigData.h"

#include "CollisionType.h"
#include "SimulationMode.h"
#include "KernelMode.h"
#include "FluidParticleLoadDenseMode.h"
#include "BlowerType.h"
#include "InsideParticleRemovalTechnique.h"
#include "RecordMode.h"
#include "LayeringGenerationTechnique.h"


Storm::GeneralSimulationData::GeneralSimulationData() :
	_gravity{ 0.f, -9.81f, 0.f },
	_particleRadius{ 0.05f },
	_kernelCoefficient{ 4.f },
	_cflCoeff{ 0.4f },
	_maxCFLIteration{ 2 },
	_startPaused{ false },
	_physicsTimeInSec{ -1.f },
	_expectedFps{ -1.f },
	_simulationMode{ Storm::SimulationMode::None },
	_minPredictIteration{ 1 },
	_maxPredictIteration{ 150 },
	_maxDensityError{ 0.01f },
	_kernelMode{ Storm::KernelMode::CubicSpline },
	_maxCFLTime{ 0.5f },
	_recomputeNeighborhoodStep{ 1 },
	_simulationNoWait{ false },
	_removeFluidParticleCollidingWithRb{ true },
	_hasFluid{ true },
	_computeCFL{ false },
	_fixRigidBodyAtStartTime{ false },
	_endSimulationPhysicsTimeInSeconds{ -1.f }
{

}

Storm::RigidBodySceneData::RigidBodySceneData() :
	_rigidBodyID{ std::numeric_limits<decltype(_rigidBodyID)>::max() },
	_static{ true },
	_isWall{ false },
	_translation{ 0.f, 0.f, 0.f },
	_rotation{ 0.f, 0.f, 0.f },
	_scale{ 1.f, 1.f, 1.f },
	_collisionShape{ Storm::CollisionType::None },
	_insideRbFluidDetectionMethodEnum{ Storm::InsideParticleRemovalTechnique::None },
	_staticFrictionCoefficient{ 0.1f },
	_dynamicFrictionCoefficient{ 0.f },
	_restitutionCoefficient{ 0.1f },
	_mass{ -1.f },
	_viscosity{ 0.f },
	_layerCount{ 1 },
	_layerGenerationMode{ Storm::LayeringGenerationTechnique::Scaling }
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
	_valueForMaxColor{ 100.f },
	_blowerAlpha{ 0.25f },
	//_constraintThickness{}, => Defined from Storm::GeneralSimulationData::_particleRadius
	_constraintColor{ 1.f, 0.1f, 0.1f, 0.8f },
	// _forceThickness{}, => Defined from Storm::GeneralSimulationData::_particleRadius
	_forceColor{ 0.f, 1.f, 1.f, 0.8f }
{

}

Storm::FluidBlockData::FluidBlockData() :
	_firstPoint{ Vector3::Zero() },
	_secondPoint{ Vector3::Zero() }, 
	_loadDenseMode{ Storm::FluidParticleLoadDenseMode::Normal }
{

}

Storm::FluidData::FluidData() :
	_fluidId{ std::numeric_limits<decltype(_fluidId)>::max() },
	_density{ 1.2754f }, // Dry air density at 0 °C degrees and normal ATM pressure. https://en.wikipedia.org/wiki/Density_of_air.
	_dynamicViscosity{ 0.00001715f }, // Dry air dynamic viscosity at 0 °C degrees and normal ATM pressure. https://www.engineeringtoolbox.com/air-absolute-kinematic-viscosity-d_601.html.
	_soundSpeed{ 331.4f }, // Sound speed in air at 0 °C degrees and normal ATM pressure. https://www.engineeringtoolbox.com/air-speed-sound-d_603.html,
	_kPressureStiffnessCoeff{ 50000.f },
	_kPressureExponentCoeff{ 7.f },
	_relaxationCoefficient{ 0.5f },
	_pressureInitRelaxationCoefficient{ 0.5f },
	_gravityEnabled{ true },
	_cinematicViscosity{ 0.f } // Computed automatically once final _dynamicViscosity value will be determined.
{

}

Storm::BlowerData::BlowerData() :
	_blowerId{ std::numeric_limits<decltype(_blowerId)>::max() },
	_startTimeInSeconds{ 0.f },
	_stopTimeInSeconds{ -1.f },
	_fadeInTimeInSeconds{ 0.f },
	_fadeOutTimeInSeconds{ 0.f },
	_blowerType{ Storm::BlowerType::None },
	_blowerDimension{ Storm::Vector3::Zero() },
	_radius{ 0.f },
	_downRadius{ -1.f },
	_upRadius{ -1.f },
	_height{ 0.f },
	_blowerPosition{ Storm::Vector3::Zero() },
	_blowerForce{ Storm::Vector3::Zero() }
{

}

Storm::ConstraintData::ConstraintData() :
	_rigidBodyId1{ std::numeric_limits<decltype(_rigidBodyId1)>::max() },
	_rigidBodyId2{ std::numeric_limits<decltype(_rigidBodyId2)>::max() },
	_constraintsLength{ 0.f },
	_rigidBody1LinkTranslationOffset{ Storm::Vector3::Zero() },
	_rigidBody2LinkTranslationOffset{ Storm::Vector3::Zero() },
	_shouldVisualize{ true },
	_preventRotations{ true }
{

}

Storm::RecordConfigData::RecordConfigData() :
	_recordMode{ Storm::RecordMode::None },
	_recordFps{ -1.f },
	_recordFilePath{},
	_replayRealTime{ true }
{

}

Storm::SceneData::SceneData() :
	_generalSimulationData{ std::make_unique<Storm::GeneralSimulationData>() },
	_graphicData{ std::make_unique<Storm::GraphicData>() },
	_fluidData{ std::make_unique<Storm::FluidData>() },
	_recordConfigData{ std::make_unique<Storm::RecordConfigData>() }
{

}

// Needed for prototypes. Otherwise, std::vector declared inside this structure won't compile anywhere else because the underlying structure wasn't defined (vector cannot destroy undefined element)...
Storm::SceneData::~SceneData() = default;
