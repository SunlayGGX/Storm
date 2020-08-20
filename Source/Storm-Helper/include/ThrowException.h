#pragma once

#include "UniversalString.h"


namespace Storm
{
	namespace details
	{
		template<class ExceptionType, class ... ArgType>
		[[noreturn]] auto throwExceptionImpl(int, int, ArgType &&... arg) -> decltype(ExceptionType{ std::forward<ArgType>(arg)... }, void())
		{
			throw ExceptionType{ std::forward<ArgType>(arg)... };
		}

		template<class ExceptionType, class StringType>
		[[noreturn]] auto throwExceptionImpl(void*, int, const StringType &msg) -> decltype(ExceptionType{ msg.c_str() }, void())
		{
			throw ExceptionType{ msg.c_str() };
		}

		template<class ExceptionType, class StringType>
		[[noreturn]] auto throwExceptionImpl(void*, void*, const StringType &msg) -> decltype(Storm::details::throwExceptionImpl<ExceptionType>(0, 0, Storm::toStdString(msg)), void())
		{
			Storm::details::throwExceptionImpl<ExceptionType>(0, 0, Storm::toStdString(msg));
		}
	}

	template<class ExceptionType, class ... ArgType>
	[[noreturn]] void throwException(ArgType &&... arg)
	{
		Storm::details::throwExceptionImpl<ExceptionType>(0, 0, std::forward<ArgType>(arg)...);
	}
}
