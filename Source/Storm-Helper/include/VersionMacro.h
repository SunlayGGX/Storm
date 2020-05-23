#pragma once


#define STORM_MAJOR 1
#define STORM_MINOR 0
#define STORM_SUBMINOR 0

#ifndef STORM_STRINGIFY
#	define STORM_STRINGIFY(val) #val
#endif

#ifndef STORM_STRINGIFY_UNDERLYING_MACRO_VALUE
#	define STORM_STRINGIFY_UNDERLYING_MACRO_VALUE(val) STORM_STRINGIFY(val)
#endif

#define STORM_VERSION_STR STORM_STRINGIFY_UNDERLYING_MACRO_VALUE(STORM_MAJOR) "." STORM_STRINGIFY_UNDERLYING_MACRO_VALUE(STORM_MINOR) "." STORM_STRINGIFY_UNDERLYING_MACRO_VALUE(STORM_SUBMINOR)
