#pragma once

#if defined(_WIN32)
namespace
{
	// Hijacking is a huge optimization, but it is really dirty...
	struct StdHijackerProxy
	{
		uint64_t _newAskedSize;
	};
}
#endif

#if defined(_WIN32)
// We hijack std::string append
template<>
template<>
std::string& std::string::append<StdHijackerProxy, 0>(const StdHijackerProxy &hijacker)
{
	this->reserve(hijacker._newAskedSize + 1);
	_Mypair._Myval2._Mysize = hijacker._newAskedSize;
	*(this->data() + _Mypair._Myval2._Mysize) = static_cast<std::string::value_type>('\0');
	return *this;
}

#endif

namespace Storm
{
	namespace
	{
		template<class Type>
		void resize_hijack(Type &inOutStr, uint64_t newSize)
		{
#if defined(_WIN32)
			StdHijackerProxy hijacker{ newSize };
			inOutStr.append<StdHijackerProxy, 0>(hijacker);
#else
			// Linux or any other platform could not have been developped their std::string the same way Windows did (variable naming, method naming, ...)
			// Since I don't have the time to check on those platform how to hijack it, I use the old resize... But this could lead to a huge overhead since
			// we're initializing the string to a value that would be overwritten just after 
			// (a useless memset of a huge string that could have been avoided since we're providing the data afterwards)...
			inOutStr.resize(newSize);
#endif
		}
	}
}
