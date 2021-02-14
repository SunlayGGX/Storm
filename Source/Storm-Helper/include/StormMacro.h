#pragma once


#ifndef STRINGIFY
#	define STRINGIFY(x) #x
#endif

#ifndef STORM_STRINGIFY
#	define STORM_STRINGIFY(x) STRINGIFY(x)
#endif

#ifndef STRINGIFY_ARGS
#	define STRINGIFY_ARGS(...) #__VA_ARGS__
#endif

#ifndef STORM_STRINGIFY_ARGS
#	define STORM_STRINGIFY_ARGS(...) STRINGIFY_ARGS(__VA_ARGS__)
#endif

#ifndef CONCAT
#	define CONCAT(x, y) x##y
#endif

#ifndef STORM_CONCAT
#	define STORM_CONCAT(x, y) CONCAT(x, y)
#endif

#ifndef STORM_MAKE_PACKED_PARAMETER
#	define STORM_MAKE_PACKED_PARAMETER(...) __VA_ARGS__
#endif


#if defined(DEBUG) || defined(_DEBUG)
#	define STORM_PLUGIN_NAME(plugin) plugin "_d"
#else
#	define STORM_PLUGIN_NAME(plugin) plugin
#endif


#define STORM_STATIC_LIBRARY_NAME(name) STORM_PLUGIN_NAME(name) ".lib"
#define STORM_DYNAMIC_LIBRARY_NAME(name) STORM_PLUGIN_NAME(name) ".dll"
#define STORM_EXECUTABLE_NAME(name) STORM_PLUGIN_NAME(name) ".exe"

#define STORM_GEN_STRUCT_NAME(rootName) STORM_CONCAT(rootName, __LINE__)

// Not implemented throwing macros.

// This throws but can be recovered with a try catch block.
#define STORM_NOT_IMPLEMENTED Storm::throwException<Storm::Exception>(__FUNCSIG__ " is not implemented!")

// The difference with STORM_NOT_IMPLEMENTED is that this provide an unrecoverable error.
#define STORM_STRONG_NOT_IMPLEMENTED struct STORM_GEN_STRUCT_NAME(_)										\
{																											\
	constexpr STORM_GEN_STRUCT_NAME(_)(const std::string_view funcSignature) : _sig{ funcSignature } {}		\
	[[noreturn]]~STORM_GEN_STRUCT_NAME(_)() 																\
	{																										\
		LOG_FATAL << _sig + " is not implemented!";															\
		std::this_thread::sleep_for(std::chrono::milliseconds{ 500 });										\
		Storm::throwException<std::logic_error>(_sig + " is not implemented!"); 							\
	}																										\
	const std::string_view _sig;  																			\
} _{ __FUNCSIG__ }


#if UNICODE
#	define STORM_TEXT(val) STORM_CONCAT(L, val)
#else
#	define STORM_TEXT(val) val
#endif


#ifdef __has_include
#	define STORM_HAS_INCLUDE(includeName) __has_include(includeName)
#else
#	define STORM_HAS_INCLUDE(includeName) true
#endif