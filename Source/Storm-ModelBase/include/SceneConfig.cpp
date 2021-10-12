#include "SceneConfig.h"

#include "SceneSimulationConfig.h"
#include "ScenePhysicsConfig.h"
#include "SceneRigidBodyConfig.h"
#include "SceneGraphicConfig.h"
#include "SceneFluidConfig.h"
#include "SceneBlowerConfig.h"
#include "SceneConstraintConfig.h"
#include "SceneRecordConfig.h"
#include "SceneScriptConfig.h"
#include "SceneFluidCustomDFSPHConfig.h"
#include "SceneFluidCustomPCISPHConfig.h"
#include "SceneFluidCustomIISPHConfig.h"
#include "SceneCageConfig.h"

#include "CollisionType.h"
#include "ConstraintType.h"
#include "SimulationMode.h"
#include "KernelMode.h"
#include "FluidParticleLoadDenseMode.h"
#include "BlowerType.h"
#include "InsideParticleRemovalTechnique.h"
#include "RecordMode.h"
#include "LayeringGenerationTechnique.h"
#include "ViscosityMethod.h"
#include "VolumeComputationTechnique.h"
#include "ParticleRemovalMode.h"


namespace
{
	inline Storm::Vector3 dummyMandatoryVector3ForMax()
	{
		return Storm::Vector3{
			std::numeric_limits<Storm::Vector3::Scalar>::lowest(),
			std::numeric_limits<Storm::Vector3::Scalar>::lowest(),
			std::numeric_limits<Storm::Vector3::Scalar>::lowest()
		};
	}

	inline Storm::Vector3 dummyMandatoryVector3ForMin()
	{
		return Storm::Vector3{
			std::numeric_limits<Storm::Vector3::Scalar>::max(),
			std::numeric_limits<Storm::Vector3::Scalar>::max(),
			std::numeric_limits<Storm::Vector3::Scalar>::max()
		};
	}
}


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
	_kernelMode{ Storm::KernelMode::CubicSpline },
	_kernelIncrementSpeedInSeconds{ -1.f },
	_maxKernelIncrementCoeff{ 0.f },
	_maxCFLTime{ 0.5f },
	_recomputeNeighborhoodStep{ 1 },
	_midUpdateViscosity{ false },
	_simulationNoWait{ false },
	_hasFluid{ true },
	_computeCFL{ false },
	_fixRigidBodyAtStartTime{ false },
	_endSimulationPhysicsTimeInSeconds{ -1.f },
	_fluidViscoMethod{ Storm::ViscosityMethod::Standard },
	_rbViscoMethod{ Storm::ViscosityMethod::Standard },
	_shouldRemoveRbCollidingPAtStateFileLoad{ true },
	_considerRbWallAtCollingingPStateFileLoad{ true },
	_fluidParticleRemovalMode{ Storm::ParticleRemovalMode::Sphere },
	_removeFluidForVolumeConsistency{ false },
	_freeRbAtPhysicsTime{ -1.f },
	_noStickConstraint{ false },
	_applyDragEffect{ false },
	_useCoendaEffect{ false }
{

}

Storm::ScenePhysicsConfig::ScenePhysicsConfig() :
	_enablePCM{ true },
	_enableAdaptiveForce{ true },
	_enableFrictionEveryIteration{ true },
	_enableStabilization{ false },
	_enableKinematicPairs{ true },
	_enableKinematicStaticPairs{ true },
	_enableAveragePoint{ true },
	_enableEnhancedDeterminism{ false },
	_enableCCD{ true },
	_removeDamping{ false }
{

}

Storm::SceneRigidBodyConfig::SceneRigidBodyConfig() :
	_rigidBodyID{ std::numeric_limits<decltype(_rigidBodyID)>::max() },
	_static{ true },
	_isWall{ false },
	_translation{ 0.f, 0.f, 0.f },
	_rotation{ Storm::Rotation::Identity() },
	_scale{ 1.f, 1.f, 1.f },
	_color{ 0.3f, 0.5f, 0.5f, 1.f },
	_collisionShape{ Storm::CollisionType::None },
	_insideRbFluidDetectionMethodEnum{ Storm::InsideParticleRemovalTechnique::None },
	_staticFrictionCoefficient{ 0.1f },
	_dynamicFrictionCoefficient{ 0.f },
	_restitutionCoefficient{ 0.1f },
	_angularVelocityDamping{ 0.05f },
	_isTranslationFixed{ false },
	_fixedSimulationVolume{ false },
	_mass{ -1.f },
	_reducedVolumeCoeff{ 0.f }, // whatever. Will be init to the one from the fluid anyway _reducedMassCoeff
	_viscosity{ 0.f },
	_noStickCoeff{ 1.f },
	_dragCoefficient{ 0.f },
	_coendaCoefficient{ 0.f },
	_layerCount{ 1 },
	_layerGenerationMode{ Storm::LayeringGenerationTechnique::Scaling },
	_volumeComputationTechnique{ Storm::VolumeComputationTechnique::None },
	_geometry{ nullptr },
	_enforceNormalsCoherency{ true }
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
	_forceColor{ 0.f, 1.f, 1.f, 0.8f },
	_normalsColor{ 1.f, 0.f, 1.f, 0.8f },
	_rbWatchId{ std::numeric_limits<std::size_t>::max() }
{

}

Storm::SceneFluidBlockConfig::SceneFluidBlockConfig() :
	_firstPoint{ Vector3::Zero() },
	_secondPoint{ Vector3::Zero() }, 
	_loadDenseMode{ Storm::FluidParticleLoadDenseMode::Normal }
{

}

Storm::SceneFluidUnitParticleConfig::SceneFluidUnitParticleConfig() :
	_position{ Vector3::Zero() }
{

}

Storm::SceneFluidCustomPCISPHConfig::SceneFluidCustomPCISPHConfig() :
	_minPredictIteration{ 2 },
	_maxPredictIteration{ 150 },
	_maxError{ 0.01f }
{

}

Storm::SceneFluidCustomIISPHConfig::SceneFluidCustomIISPHConfig() :
	_minPredictIteration{ 2 },
	_maxPredictIteration{ 150 },
	_maxError{ 0.01f }
{

}

Storm::SceneFluidCustomDFSPHConfig::SceneFluidCustomDFSPHConfig() :
	_neighborThresholdDensity{ 20 },
	_kPressurePredictedCoeff{ 1.f },
	_enableThresholdDensity{ true },
	_useFixRotation{ true },
	_enableDensitySolve{ true },
	_useBernoulliPrinciple{ false },
	_minPredictIteration{ 2 },
	_maxPredictIteration{ 150 },
	_maxDensityError{ 0.01f },
	_maxPressureError{ 0.01f }
{

}

Storm::SceneFluidConfig::SceneFluidConfig() :
	_fluidId{ std::numeric_limits<decltype(_fluidId)>::max() },
	_density{ 1.2754f }, // Dry air density at 0 °C degrees and normal ATM pressure. https://en.wikipedia.org/wiki/Density_of_air.
	_dynamicViscosity{ 0.00001715f }, // Dry air dynamic viscosity at 0 °C degrees and normal ATM pressure. https://www.engineeringtoolbox.com/air-absolute-kinematic-viscosity-d_601.html.
	_soundSpeed{ 331.4f }, // Sound speed in air at 0 °C degrees and normal ATM pressure. https://www.engineeringtoolbox.com/air-speed-sound-d_603.html,
	_particleVolume{ -1.f },
	_kPressureStiffnessCoeff{ -1.f },
	_kPressureExponentCoeff{ 7.f },
	_relaxationCoefficient{ 0.5f },
	_pressureInitRelaxationCoefficient{ 0.5f },
	_gravityEnabled{ true },
	_removeParticlesCollidingWithRb{ true },
	_removeOutDomainParticles{ true },
	_customSimulationSettings{ nullptr }, // This will be filled when we'll read the config file to the right settings structure depending on our simulation mode.
	_cinematicViscosity{ 0.f }, // Computed automatically once final _dynamicViscosity value will be determined.
	_uniformDragCoefficient{ 0.f },
	_reducedMassCoefficient{ 0.8f }
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
	_type{ Storm::ConstraintType::None },
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

Storm::SceneCageConfig::SceneCageConfig() :
	_boxMin{ dummyMandatoryVector3ForMin() },
	_boxMax{ dummyMandatoryVector3ForMax() },
	_infiniteDomain{ false },
	_passthroughVelReduceCoeff{ 1.f, 1.f, 1.f }
{

}

Storm::SceneConfig::SceneConfig() = default;

// Needed for prototypes. Otherwise, std::vector declared inside this structure won't compile anywhere else because the underlying structure wasn't defined (vector cannot destroy undefined element)...
Storm::SceneConfig::~SceneConfig() = default;
