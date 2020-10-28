#pragma once


#if defined(__AVX__) || defined(__AVX2__) || defined(_M_X64)
#	define STORM_USE_INTRINSICS true

#	if false
#		define STORM_INTRINSICS_LOAD_PS_FROM_VECT3(vect3) _mm_setr_ps(vect3.x(), vect3.y(), vect3.z(), 0.f)
#	else
#		define STORM_INTRINSICS_LOAD_PS_FROM_VECT3(vect3) _mm_loadu_ps(reinterpret_cast<const float*>(&vect3[0]))
#	endif
#else
#	define STORM_USE_INTRINSICS false
#endif
