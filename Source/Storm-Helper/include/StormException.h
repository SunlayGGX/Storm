#pragma once

#include <exception>


namespace Storm
{
	class StormException : public std::exception
	{
	public:
		using _Mybase = std::exception;
		using ExceptionDataType = __std_exception_data;

	public:
		StormException(const char*const exceptionMsg) noexcept;
		StormException(const std::string_view &exceptionMsg) noexcept;
		StormException(const std::string &exceptionMsg) noexcept;
		StormException(const Storm::StormException &other) noexcept;

		virtual ~StormException() noexcept;

	public:
		Storm::StormException& operator=(const Storm::StormException &other) noexcept;

	public:
		_NODISCARD const std::string_view stackTrace() const noexcept;

	protected:
		ExceptionDataType _stackTrace;
		std::size_t _stackTraceSize;
	};
}
