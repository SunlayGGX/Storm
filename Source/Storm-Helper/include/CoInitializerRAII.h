#pragma once

#include "LeanWindowsInclude.h"


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

