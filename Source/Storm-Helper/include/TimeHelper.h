#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	class TimeHelper : private Storm::NonInstanciable
	{
	public:
		static std::string_view getWeekDay(int tmWday);
		static std::string getCurrentDate();
		static std::string getCurrentDateTime(bool considerMillisec);
	};
}
