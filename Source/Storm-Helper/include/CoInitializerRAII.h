#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#   include <Windows.h>
#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN

namespace Storm
{
	class CoInitializerRAII
	{
	public:
		CoInitializerRAII();
		CoInitializerRAII(DWORD flag);
		~CoInitializerRAII();

		HRESULT getCoInitializeCallResult() const;

	public:
		bool _shouldCoUninitialize;

	private:
		HRESULT _result;
	};
}

