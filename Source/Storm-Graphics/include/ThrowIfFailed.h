#pragma once

#include <comdef.h>


namespace Storm
{
	inline void throwIfFailedImpl(HRESULT res, const std::string &callExpression)
	{
		if (!SUCCEEDED(res))
		{
			Storm::throwException<Storm::StormException>(callExpression + " failed! Error was " + Storm::toStdString(_com_error{ res }));
		}
	}

#define throwIfFailed(expression) throwIfFailedImpl(expression, #expression " in function " __FUNCTION__)
}