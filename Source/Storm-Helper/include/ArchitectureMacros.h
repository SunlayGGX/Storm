#pragma once


#if defined(__AVX__) || defined(__AVX2__) || defined(_M_X64)
#	define STORM_USE_INTRINSICS true
#else
#	define STORM_USE_INTRINSICS false
#endif
