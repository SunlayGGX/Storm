#pragma once


#include "LuaScriptWrapper.h"

#include "NameExtractor.h"

#include "SimulatorManager.h"
#include "TimeManager.h"
#include "GraphicManager.h"
#include "OSManager.h"
#include "PhysicsManager.h"

#include "ColoredSetting.h"
#include "GraphicCutMode.h"
#include "CustomForceSelect.h"


/////////////////////////////////////////////////
/////////////// Utility Macros //////////////////
/////////////////////////////////////////////////

#ifdef registerCurrentInstance
#	define STORM_registerCurrentInstance_DEFINED
#	pragma push_macro("registerCurrentInstance")
#	undef registerCurrentInstance
#endif

#ifdef registerCurrentType
#	define STORM_registerCurrentType_DEFINED
#	pragma push_macro("registerCurrentType")
#	undef registerCurrentType
#endif

#ifdef registerCurrentEnum
#	define STORM_registerCurrentEnum_DEFINED
#	pragma push_macro("registerCurrentEnum")
#	undef registerCurrentEnum
#endif


#define registerCurrentInstance(instanceName) registerInstance(this, instanceName)
#define registerCurrentType(...) registerType<STORM_CURRENT_REGISTERED_TYPE>(std::string{ Storm::NameExtractor::extractTypeNameFromType(STORM_STRINGIFY(STORM_CURRENT_REGISTERED_TYPE)) }, __VA_ARGS__)
#define registerCurrentEnum(EnumType, ...) registerEnum(Storm::NameExtractor::extractTypeNameFromType(STORM_STRINGIFY(EnumType)), __VA_ARGS__)

#define STORM_DECLARE_SCRIPTED_METHOD(methodName) #methodName, &STORM_CURRENT_REGISTERED_TYPE::methodName
#define STORM_DECLARE_SCRIPTED_ENUM(enumValue) Storm::NameExtractor::extractTypeNameFromType(STORM_STRINGIFY(enumValue)), enumValue



/////////////////////////////////////////////////
/////////////// IMPLEMENTATIONS /////////////////
/////////////////////////////////////////////////


//---------------------------------------------------------------------

#define STORM_CURRENT_REGISTERED_TYPE Storm::SimulatorManager

template<class IScriptWrapperInterface>
void STORM_CURRENT_REGISTERED_TYPE::registerCurrentOnScript(IScriptWrapperInterface &script) const
{
	script
		.registerCurrentEnum(Storm::CustomForceSelect,
			STORM_DECLARE_SCRIPTED_ENUM(Storm::CustomForceSelect::Pressure),
			STORM_DECLARE_SCRIPTED_ENUM(Storm::CustomForceSelect::Viscosity),
			STORM_DECLARE_SCRIPTED_ENUM(Storm::CustomForceSelect::Drag),
			STORM_DECLARE_SCRIPTED_ENUM(Storm::CustomForceSelect::Bernouilli),
			STORM_DECLARE_SCRIPTED_ENUM(Storm::CustomForceSelect::NoStick),
			STORM_DECLARE_SCRIPTED_ENUM(Storm::CustomForceSelect::Coenda),
			STORM_DECLARE_SCRIPTED_ENUM(Storm::CustomForceSelect::AllPressure),
			STORM_DECLARE_SCRIPTED_ENUM(Storm::CustomForceSelect::AllViscosity),
			STORM_DECLARE_SCRIPTED_ENUM(Storm::CustomForceSelect::AllDrag),
			STORM_DECLARE_SCRIPTED_ENUM(Storm::CustomForceSelect::AllBernouilli),
			STORM_DECLARE_SCRIPTED_ENUM(Storm::CustomForceSelect::AllNoStick),
			STORM_DECLARE_SCRIPTED_ENUM(Storm::CustomForceSelect::AllCoenda)
		).registerCurrentType(
		
		STORM_DECLARE_SCRIPTED_METHOD(resetReplay),

		STORM_DECLARE_SCRIPTED_METHOD(saveSimulationState),

		// Solver manip
		STORM_DECLARE_SCRIPTED_METHOD(setEnableThresholdDensity_DFSPH),
		STORM_DECLARE_SCRIPTED_METHOD(setNeighborThresholdDensity_DFSPH),
		STORM_DECLARE_SCRIPTED_METHOD(setUseRotationFix_DFSPH),


		// Frame advance manip
		STORM_DECLARE_SCRIPTED_METHOD(advanceOneFrame),
		STORM_DECLARE_SCRIPTED_METHOD(advanceByFrame),
		STORM_DECLARE_SCRIPTED_METHOD(advanceToFrame),

		// General
		STORM_DECLARE_SCRIPTED_METHOD(setGravityEnabled),
		STORM_DECLARE_SCRIPTED_METHOD(selectSpecificParticle),
		STORM_DECLARE_SCRIPTED_METHOD(selectRigidbodyToDisplayNormals),
		STORM_DECLARE_SCRIPTED_METHOD(clearRigidbodyToDisplayNormals),
		STORM_DECLARE_SCRIPTED_METHOD(selectCustomForcesDisplay),
		
		// Debug
		STORM_DECLARE_SCRIPTED_METHOD(printRigidBodyMoment),
		STORM_DECLARE_SCRIPTED_METHOD(printRigidBodyGlobalDensity),
		STORM_DECLARE_SCRIPTED_METHOD(printFluidParticleData),
		STORM_DECLARE_SCRIPTED_METHOD(printMassForRbDensity),
		STORM_DECLARE_SCRIPTED_METHOD(logAverageDensity),
		STORM_DECLARE_SCRIPTED_METHOD(logVelocityData),
		STORM_DECLARE_SCRIPTED_METHOD(logTotalVolume),
		STORM_DECLARE_SCRIPTED_METHOD(logSelectedParticleContributionToVelocity),
		STORM_DECLARE_SCRIPTED_METHOD(logSelectedParticleContributionToTotalForce),
		STORM_DECLARE_SCRIPTED_METHOD(logSelectedParticleContributionToVector),
		STORM_DECLARE_SCRIPTED_METHOD(logForceParticipationOnTotalForce),
		STORM_DECLARE_SCRIPTED_METHOD(logForceParticipationOnVector),
		STORM_DECLARE_SCRIPTED_METHOD(writeCurrentFrameSystemForcesToCsv),
		STORM_DECLARE_SCRIPTED_METHOD(writeParticleNeighborhood),
		STORM_DECLARE_SCRIPTED_METHOD(writeRbEmptiness),
		STORM_DECLARE_SCRIPTED_METHOD(forceRefreshParticleNeighborhood) // Yes, this is to debug the particle neighborhood regeneration. Should not be called otherwise.

	).registerCurrentInstance("simulMgr");
}

#undef STORM_CURRENT_REGISTERED_TYPE


//---------------------------------------------------------------------

#define STORM_CURRENT_REGISTERED_TYPE Storm::TimeManager

template<class IScriptWrapperInterface>
void STORM_CURRENT_REGISTERED_TYPE::registerCurrentOnScript(IScriptWrapperInterface &script) const
{
	script.registerCurrentType(

		STORM_DECLARE_SCRIPTED_METHOD(changeSimulationPauseState),

		STORM_DECLARE_SCRIPTED_METHOD(resetPhysicsElapsedTime)

	).registerCurrentInstance("timeMgr");
}

#undef STORM_CURRENT_REGISTERED_TYPE


//---------------------------------------------------------------------

#define STORM_CURRENT_REGISTERED_TYPE Storm::GraphicManager

template<class IScriptWrapperInterface>
void STORM_CURRENT_REGISTERED_TYPE::registerCurrentOnScript(IScriptWrapperInterface &script) const
{
	script.registerCurrentEnum(Storm::ColoredSetting,
		STORM_DECLARE_SCRIPTED_ENUM(Storm::ColoredSetting::Velocity),
		STORM_DECLARE_SCRIPTED_ENUM(Storm::ColoredSetting::Pressure),
		STORM_DECLARE_SCRIPTED_ENUM(Storm::ColoredSetting::Density)
	);

	script.registerCurrentEnum(Storm::GraphicCutMode,
		STORM_DECLARE_SCRIPTED_ENUM(Storm::GraphicCutMode::Kernel),
		STORM_DECLARE_SCRIPTED_ENUM(Storm::GraphicCutMode::Particle)
	);

	script.registerCurrentType(

		STORM_DECLARE_SCRIPTED_METHOD(cycleColoredSetting),
		STORM_DECLARE_SCRIPTED_METHOD(setUseColorSetting),
		STORM_DECLARE_SCRIPTED_METHOD(setColorSettingMinMaxValue),

		STORM_DECLARE_SCRIPTED_METHOD(showCoordinateSystemAxis),

		STORM_DECLARE_SCRIPTED_METHOD(lockNearPlaneOnWatchedRb),
		STORM_DECLARE_SCRIPTED_METHOD(unlockNearPlaneOnWatchedRb),

		STORM_DECLARE_SCRIPTED_METHOD(setUIFieldEnabled),

		STORM_DECLARE_SCRIPTED_METHOD(setVectMultiplicatorCoeff),

		STORM_DECLARE_SCRIPTED_METHOD(makeCutAroundWatchedRb),
		STORM_DECLARE_SCRIPTED_METHOD(makeCutAroundRigidbody),
		STORM_DECLARE_SCRIPTED_METHOD(makeCutAroundSelectedParticle)

	).registerCurrentInstance("graphicMgr");
}

#undef STORM_CURRENT_REGISTERED_TYPE


//---------------------------------------------------------------------

#define STORM_CURRENT_REGISTERED_TYPE Storm::PhysicsManager

template<class IScriptWrapperInterface>
void STORM_CURRENT_REGISTERED_TYPE::registerCurrentOnScript(IScriptWrapperInterface &script) const
{
	script.registerCurrentType(

		STORM_DECLARE_SCRIPTED_METHOD(setRigidBodyAngularDamping),
		STORM_DECLARE_SCRIPTED_METHOD(fixDynamicRigidBodyTranslation),
		STORM_DECLARE_SCRIPTED_METHOD(setRigidBodiesFixed),

		STORM_DECLARE_SCRIPTED_METHOD(reconnectPhysicsDebugger)

	).registerCurrentInstance("physicsMgr");
}

#undef STORM_CURRENT_REGISTERED_TYPE


//---------------------------------------------------------------------

#define STORM_CURRENT_REGISTERED_TYPE Storm::OSManager

template<class IScriptWrapperInterface>
void STORM_CURRENT_REGISTERED_TYPE::registerCurrentOnScript(IScriptWrapperInterface &script) const
{
	script.registerCurrentType(

		STORM_DECLARE_SCRIPTED_METHOD(clearProcesses)

	).registerCurrentInstance("osMgr");
}

#undef STORM_CURRENT_REGISTERED_TYPE


//---------------------------------------------------------------------

#define STORM_CURRENT_REGISTERED_TYPE Storm::WindowsManager

template<class IScriptWrapperInterface>
void STORM_CURRENT_REGISTERED_TYPE::registerCurrentOnScript(IScriptWrapperInterface &script) const
{
	script.registerCurrentType(

		STORM_DECLARE_SCRIPTED_METHOD(restartApplication)

	).registerCurrentInstance("winMgr");
}

#undef STORM_CURRENT_REGISTERED_TYPE



/////////////////////////////////////////////////
//////////// UNDEFINITIONS CLEANUP //////////////
/////////////////////////////////////////////////


#ifdef STORM_registerCurrentInstance_DEFINED
#	undef registerCurrentInstance
#	pragma pop_macro("registerCurrentInstance")
#	undef STORM_registerCurrentInstance_DEFINED
#endif

#ifdef STORM_registerCurrentType_DEFINED
#	undef registerCurrentType
#	pragma pop_macro("registerCurrentType")
#	undef STORM_registerCurrentType_DEFINED
#endif

#ifdef STORM_registerCurrentEnum_DEFINED
#	undef registerCurrentEnum
#	pragma pop_macro("registerCurrentEnum")
#	undef STORM_registerCurrentEnum_DEFINED
#endif

#undef STORM_METHOD
