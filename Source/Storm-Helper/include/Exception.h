#pragma once

#include <exception>


namespace Storm
{
	class Exception : public std::exception
	{
	public:
		using _Mybase = std::exception;
		using ExceptionDataType = __std_exception_data;

	public:
		Exception(const char*const exceptionMsg) noexcept;
		Exception(const std::string_view &exceptionMsg) noexcept;
		Exception(const std::string &exceptionMsg) noexcept;
		Exception(const Storm::Exception &other) noexcept;

		virtual ~Exception() noexcept;

	public:
		Storm::Exception& operator=(const Storm::Exception &other) noexcept;

	public:
		_NODISCARD const std::string_view stackTrace() const noexcept;

	protected:
		ExceptionDataType _stackTrace;
		std::size_t _stackTraceSize;
	};
}
