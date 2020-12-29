#pragma once


#include "StormStdPrerequisite.h"

#include "DebuggerHelper.h"

#include "ArchitectureMacros.h"
#include "MacroConfig.h"

#include "Exception.h"
#include "ThrowException.h"


// 4624: 'XXXXXX': destructor was implicitly defined as deleted => Yes it is intended for classes that shouldn't be instantiated (abstract classes). NonInstanciable would always fire this warning so we need to disable it.
#define STORM_STDPREREQUISITE_DISABLE_WARNING_ALL 4624
#pragma warning(disable: STORM_STDPREREQUISITE_DISABLE_WARNING_ALL)
