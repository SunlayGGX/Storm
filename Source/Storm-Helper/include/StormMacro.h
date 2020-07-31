#pragma once


#ifndef STRINGIFY
#	define STRINGIFY(x) #x
#endif

#ifndef STORM_STRINGIFY
#	define STORM_STRINGIFY(x) STRINGIFY(x)
#endif


#if defined(DEBUG) || defined(_DEBUG)
#	define STORM_PLUGIN_NAME(plugin) STORM_STRINGIFY(plugin) "_d"
#else
#	define STORM_PLUGIN_NAME(plugin) STORM_STRINGIFY(plugin)
#endif


#define STORM_STATIC_LIBRARY_NAME(name) STORM_PLUGIN_NAME(name) ".lib"
#define STORM_DYNAMIC_LIBRARY_NAME(name) STORM_PLUGIN_NAME(name) ".dll"
#define STORM_EXECUTABLE_NAME(name) STORM_PLUGIN_NAME(name) ".exe"


#define STORM_NOT_IMPLEMENTED Storm::throwException<std::logic_error>(__FUNCTION__ " is not implemented!")


#if UNICODE
#	define STORM_TEXT(val) L##val
#else
#	define STORM_TEXT(val) val
#endif