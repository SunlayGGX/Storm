#pragma once


#include "LuaScriptWrapper.h"

#include "NameExtractor.h"

#include "SimulatorManager.h"
#include "TimeManager.h"
#include "GraphicManager.h"
#include "OSManager.h"


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


#define registerCurrentInstance(instanceName) registerInstance(this, instanceName)
#define registerCurrentType(...) registerType<STORM_CURRENT_REGISTERED_TYPE>(std::string{ Storm::NameExtractor::extractTypeNameFromType(STORM_STRINGIFY(STORM_CURRENT_REGISTERED_TYPE)) }, __VA_ARGS__)

#define STORM_DECLARE_SCRIPTED_METHOD(methodName) #methodName, &STORM_CURRENT_REGISTERED_TYPE::methodName



/////////////////////////////////////////////////
/////////////// IMPLEMENTATIONS /////////////////
/////////////////////////////////////////////////


//---------------------------------------------------------------------

#define STORM_CURRENT_REGISTERED_TYPE Storm::SimulatorManager

template<class IScriptWrapperInterface>
void STORM_CURRENT_REGISTERED_TYPE::registerCurrentOnScript(IScriptWrapperInterface &script) const
{
	script.registerCurrentType(
		
		STORM_DECLARE_SCRIPTED_METHOD(resetReplay),

		STORM_DECLARE_SCRIPTED_METHOD(saveSimulationState),

		// Frame advance manip
		STORM_DECLARE_SCRIPTED_METHOD(advanceOneFrame),
		STORM_DECLARE_SCRIPTED_METHOD(advanceByFrame)

	).registerCurrentInstance("simulMgr");
}

#undef STORM_CURRENT_REGISTERED_TYPE


//---------------------------------------------------------------------

#define STORM_CURRENT_REGISTERED_TYPE Storm::TimeManager

template<class IScriptWrapperInterface>
void STORM_CURRENT_REGISTERED_TYPE::registerCurrentOnScript(IScriptWrapperInterface &script) const
{
	script.registerCurrentType(

		STORM_DECLARE_SCRIPTED_METHOD(changeSimulationPauseState)

	).registerCurrentInstance("timeMgr");
}

#undef STORM_CURRENT_REGISTERED_TYPE


//---------------------------------------------------------------------

#define STORM_CURRENT_REGISTERED_TYPE Storm::GraphicManager

template<class IScriptWrapperInterface>
void STORM_CURRENT_REGISTERED_TYPE::registerCurrentOnScript(IScriptWrapperInterface &script) const
{
	script.registerCurrentType(

		STORM_DECLARE_SCRIPTED_METHOD(cycleColoredSetting),
		STORM_DECLARE_SCRIPTED_METHOD(setColorSettingMinMaxValue),

		STORM_DECLARE_SCRIPTED_METHOD(setUIFieldEnabled)

	).registerCurrentInstance("graphicMgr");
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

#undef STORM_METHOD