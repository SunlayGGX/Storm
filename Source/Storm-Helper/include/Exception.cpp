#include "Exception.h"

#include "DebuggerHelper.h"



namespace
{
	void copyExceptionData(const Storm::Exception::ExceptionDataType &from, Storm::Exception::ExceptionDataType &to) noexcept
	{
		__std_exception_copy(&from, &to);
	}

	void destroyExceptionData(Storm::Exception::ExceptionDataType &toDestroy) noexcept
	{
		__std_exception_destroy(&toDestroy);
	}
}


Storm::Exception::Exception(const char*const exceptionMsg) noexcept :
	std::exception{ exceptionMsg },
	_stackTrace{}
{
	Storm::Exception::ExceptionDataType tmp = { nullptr, true };

	{
		const std::string currentStackTrace = Storm::obtainStackTrace(false);
		tmp._What = currentStackTrace.c_str();
		_stackTraceSize = currentStackTrace.size();

		copyExceptionData(tmp, _stackTrace);
	}
}

Storm::Exception::Exception(const std::string_view &exceptionMsg) noexcept :
	Storm::Exception{ exceptionMsg.data() }
{}

Storm::Exception::Exception(const std::string &exceptionMsg) noexcept :
	Storm::Exception{ exceptionMsg.c_str() }
{}

Storm::Exception::Exception(const Storm::Exception &other) noexcept :
	std::exception{ other.what() },
	_stackTrace{}
{
	copyExceptionData(other._stackTrace, _stackTrace);
	_stackTraceSize = other._stackTraceSize;
}

Storm::Exception::~Exception() noexcept
{
	destroyExceptionData(_stackTrace);
}

Storm::Exception& Storm::Exception::operator=(const Storm::Exception &other) noexcept
{
	if (this != &other)
	{
		destroyExceptionData(_stackTrace);

		copyExceptionData(other._stackTrace, _stackTrace);
		_stackTraceSize = other._stackTraceSize;
	}

	return *this;
}

const std::string_view Storm::Exception::stackTrace() const noexcept
{
	return std::string_view{ _stackTrace._What, _stackTraceSize };
}
