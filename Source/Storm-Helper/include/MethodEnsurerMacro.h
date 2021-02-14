#pragma once


#include "StormMacro.h"
#include "StaticAssertionsMacros.h"


#define STORM_ENSURE_HAS_METHOD(EvalType, ConvertibleRet, FuncName, ...)																											\
	template<class ... Args>																																						\
	static constexpr auto STORM_CONCAT(ensureHasMethod, __LINE__)(int)																												\
		-> decltype(std::enable_if_t<std::is_convertible_v<decltype(((EvalType*)(nullptr))->FuncName(std::declval<Args>()...)), ConvertibleRet>, std::true_type>::value)			\
	{																																												\
		return 0;																																									\
	}																																												\
	template<class ... Args>																																						\
	static constexpr int STORM_CONCAT(ensureHasMethod, __LINE__)(void*)																												\
	{																																												\
		STORM_COMPILE_ERROR(																																						\
			STORM_STRINGIFY(EvalType) " doesn't define '"																															\
			STORM_STRINGIFY(ConvertibleRet) " " STORM_STRINGIFY(FuncName) "(" STORM_STRINGIFY_ARGS(STORM_MAKE_PACKED_PARAMETER(__VA_ARGS__)) ") [specifiers]'."						\
		);																																											\
		return 0;																																									\
	}																																												\
	enum { STORM_CONCAT(k_, __LINE__) = STORM_CONCAT(ensureHasMethod, __LINE__)<__VA_ARGS__>(0) }
