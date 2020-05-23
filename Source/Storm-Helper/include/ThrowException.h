#pragma once


namespace Storm
{
	namespace details
	{
		template<class ExceptionType, class ... ArgType>
		[[noreturn]] auto throwExceptionImpl(int, ArgType &&... arg) -> decltype(ExceptionType{ std::forward<ArgType>(arg)... }, void())
		{
			throw ExceptionType{ std::forward<ArgType>(arg)... };
		}

		template<class ExceptionType, class StringType>
		[[noreturn]] auto throwExceptionImpl(void*, const StringType &msg) -> decltype(ExceptionType{ msg.c_str() }, void())
		{
			throw ExceptionType{ msg.c_str() };
		}
	}

	template<class ExceptionType, class ... ArgType>
	[[noreturn]] void throwException(ArgType &&... arg)
	{
		Storm::details::throwExceptionImpl<ExceptionType>(0, std::forward<ArgType>(arg)...);
	}
}
