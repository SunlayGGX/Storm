#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	class TimeHelper : private Storm::NonInstanciable
	{
	public:
		static std::string_view getWeekDay(const int tmWday);
		static std::string getCurrentDate();
		static std::string getCurrentDateTime(const bool considerMillisec);
	};
}
