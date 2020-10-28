#ifdef STORM_HIJACKED_TYPE

#	if _WIN32
#		include "VectorHijacker.h"

// (   - _ - |||)
// Ugly but super efficient...
template<>
template<>
inline decltype(auto) std::vector<STORM_HIJACKED_TYPE, std::allocator<STORM_HIJACKED_TYPE>>::emplace_back<Storm::VectorHijackerMakeBelieve>(Storm::VectorHijackerMakeBelieve hijacker)
{
	// We're modifying directly the size of the vector without passing by the extra initialization.
	_Mypair._Myval2._Mylast = _Mypair._Myval2._Myfirst + hijacker._newSize;
}

namespace Storm
{
	inline void setNumUninitialized_hijack(std::vector<STORM_HIJACKED_TYPE> &hijackedVector, Storm::VectorHijackerMakeBelieve hijacker)
	{
		hijackedVector.emplace_back<Storm::VectorHijackerMakeBelieve>(hijacker);
	}
}
#	endif

#else

#	error STORM_HIJACKED_TYPE should be defined!!

#endif
