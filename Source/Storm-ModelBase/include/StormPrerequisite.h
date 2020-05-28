#pragma once


#include "StormStdPrerequisite.h"


// 4624: 'XXXXXX': destructor was implicitly defined as deleted => Yes it is intended for classes that shouldn't be instantiated (abstract classes). NonInstanciable would always fire this warning so we need to disable it.
#define STORM_DISABLE_WARNING_ALL 4624
#pragma warning(disable: STORM_DISABLE_WARNING_ALL)


#include "StaticAssertionsMacros.h"

#include "StormMacro.h"

#include "Vector3.h"
