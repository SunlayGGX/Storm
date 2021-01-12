#include "SceneConfig.h"

#include "SceneSimulationConfig.h"
#include "SceneRigidBodyConfig.h"
#include "SceneGraphicConfig.h"
#include "SceneFluidConfig.h"
#include "SceneBlowerConfig.h"
#include "SceneConstraintConfig.h"
#include "SceneRecordConfig.h"
#include "SceneScriptConfig.h"

#include "CollisionType.h"
#include "SimulationMode.h"
#include "KernelMode.h"
#include "FluidParticleLoadDenseMode.h"
#include "BlowerType.h"
#include "InsideParticleRemovalTechnique.h"
#include "RecordMode.h"
#include "LayeringGenerationTechnique.h"


Storm::SceneSimulationConfig::SceneSimulationConfig() :
	_gravity{ 0.f, -9.81f, 0.f },
	_particleRadius{ 0.05f },
	_kernelCoefficient{ 4.f },
	_cflCoeff{ 0.4f },
	_maxCFLIteration{ 2 },
	_startPaused{ false },
	_physicsTimeInSec{ -1.f },
	_expectedFps{ -1.f },
	_simulationMode{ Storm::SimulationMode::None },
	_minPredictIteration{ 2 },
	_maxPredictIteration{ 150 },
	_maxDensityError{ 0.01f },
	_kernelMode{ Storm::KernelMode::CubicSpline },
	_kernelIncrementSpeedInSeconds{ -1.f },
	_maxKernelIncrementCoeff{ 0.f },
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

Storm::SceneRigidBodyConfig::SceneRigidBodyConfig() :
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
	_angularVelocityDamping{ 0.05f },
	_isTranslationFixed{ false },
	_mass{ -1.f },
	_viscosity{ 0.f },
	_layerCount{ 1 },
	_layerGenerationMode{ Storm::LayeringGenerationTechnique::Scaling }
{

}

Storm::SceneGraphicConfig::SceneGraphicConfig() :
	_cameraPosition{ 0.f, 0.f, -10.f },
	_cameraLookAt{ 0.f, 0.f, 0.f },
	_zNear{ 0.01f },
	_zFar{ 20.f },
	_grid{ 10.f, 0.f, 10.f },
	_displaySolidAsParticles{ false },
	_velocityNormMinColor{ 0.01f },
	_velocityNormMaxColor{ 100.f },
	_pressureMinColor{ 0.f },
	_pressureMaxColor{ 10000.f },
	_densityMinColor{ 0.f },
	_densityMaxColor{ 2000.f },
	_blowerAlpha{ 0.25f },
	//_constraintThickness{}, => Defined from Storm::SceneSimulationConfig::_particleRadius
	_constraintColor{ 1.f, 0.1f, 0.1f, 0.8f },
	// _forceThickness{}, => Defined from Storm::SceneSimulationConfig::_particleRadius
	_forceColor{ 0.f, 1.f, 1.f, 0.8f }
{

}

Storm::SceneFluidBlockConfig::SceneFluidBlockConfig() :
	_firstPoint{ Vector3::Zero() },
	_secondPoint{ Vector3::Zero() }, 
	_loadDenseMode{ Storm::FluidParticleLoadDenseMode::Normal }
{

}

Storm::SceneFluidConfig::SceneFluidConfig() :
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

Storm::SceneBlowerConfig::SceneBlowerConfig() :
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

Storm::SceneConstraintConfig::SceneConstraintConfig() :
	_rigidBodyId1{ std::numeric_limits<decltype(_rigidBodyId1)>::max() },
	_rigidBodyId2{ std::numeric_limits<decltype(_rigidBodyId2)>::max() },
	_constraintsLength{ 0.f },
	_rigidBody1LinkTranslationOffset{ Storm::Vector3::Zero() },
	_rigidBody2LinkTranslationOffset{ Storm::Vector3::Zero() },
	_shouldVisualize{ true },
	_preventRotations{ true }
{

}

Storm::SceneRecordConfig::SceneRecordConfig() :
	_recordMode{ Storm::RecordMode::None },
	_recordFps{ -1.f },
	_recordFilePath{},
	_replayRealTime{ true }
{

}

Storm::SceneScriptConfig::SceneScriptConfig() :
	_enabled{ true }
{
	_scriptFilePipe._refreshRateInMillisec = 100;

	_scriptFilePipe._filePath = (std::filesystem::path{ "$[StormScripts]" } / "RuntimeScript.txt").string();
}

// Needed for prototypes. Otherwise, std::vector declared inside this structure won't compile anywhere else because the underlying structure wasn't defined (vector cannot destroy undefined element)...
Storm::SceneConfig::~SceneConfig() = default;
