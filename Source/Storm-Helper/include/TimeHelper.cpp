#include "TimeHelper.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#	include <Windows.h>
#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN


namespace
{
	template<class Type>
	std::string makeATwoNumber(Type val)
	{
		return val < 10 ? ('0' + std::to_string(val)) : std::to_string(val);
	}
}


std::string_view Storm::TimeHelper::getWeekDay(int tmWday)
{
	switch (tmWday)
	{
	case 0: return "Sunday";
	case 1: return "Monday";
	case 2: return "Tuesday";
	case 3: return "Wednesday";
	case 4: return "Thursday";
	case 5: return "Friday";
	case 6: return "Saturday";
	default: return "";
	}
}

std::string Storm::TimeHelper::getCurrentDate()
{
	std::string result;
	result.reserve(32);

	SYSTEMTIME currentTm;
	::GetSystemTime(&currentTm);

	result += getWeekDay(currentTm.wDay);
	result += '_';
	result += std::to_string(currentTm.wYear + 1900);
	result += '_';
	result += makeATwoNumber(currentTm.wMonth);
	result += '_';
	result += makeATwoNumber(currentTm.wDay);

	return result;
}

std::string Storm::TimeHelper::getCurrentDateTime(bool considerMillisec)
{
	std::string result;
	result.reserve(32);

	SYSTEMTIME currentTm;
	::GetSystemTime(&currentTm);

	result += Storm::TimeHelper::getWeekDay(currentTm.wDay);
	result += '_';
	result += std::to_string(currentTm.wYear + 1900);
	result += '_';
	result += makeATwoNumber(currentTm.wMonth);
	result += '_';
	result += makeATwoNumber(currentTm.wDay);
	result += '_';
	result += makeATwoNumber(currentTm.wHour);
	result += '_';
	result += makeATwoNumber(currentTm.wMinute);
	result += '_';
	result += makeATwoNumber(currentTm.wSecond);

	if (considerMillisec)
	{
		result += '_';
		result += std::to_string(currentTm.wMilliseconds);
	}

	return result;
}

