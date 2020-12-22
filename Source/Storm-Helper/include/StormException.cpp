#include "StormException.h"

#include "DebuggerHelper.h"



namespace
{
	void copyExceptionData(const Storm::StormException::ExceptionDataType &from, Storm::StormException::ExceptionDataType &to) noexcept
	{
		__std_exception_copy(&from, &to);
	}

	void destroyExceptionData(Storm::StormException::ExceptionDataType &toDestroy) noexcept
	{
		__std_exception_destroy(&toDestroy);
	}
}


Storm::StormException::StormException(const char*const exceptionMsg) noexcept :
	std::exception{ exceptionMsg },
	_stackTrace{}
{
	Storm::StormException::ExceptionDataType tmp = { nullptr, true };

	{
		const std::string currentStackTrace = Storm::obtainStackTrace(false);
		tmp._What = currentStackTrace.c_str();
		_stackTraceSize = currentStackTrace.size();

		copyExceptionData(tmp, _stackTrace);
	}
}

Storm::StormException::StormException(const std::string_view &exceptionMsg) noexcept :
	Storm::StormException{ exceptionMsg.data() }
{}

Storm::StormException::StormException(const std::string &exceptionMsg) noexcept :
	Storm::StormException{ exceptionMsg.c_str() }
{}

Storm::StormException::StormException(const Storm::StormException &other) noexcept
{
	copyExceptionData(other._stackTrace, _stackTrace);
	_stackTraceSize = other._stackTraceSize;
}

Storm::StormException::~StormException() noexcept
{
	destroyExceptionData(_stackTrace);
}

Storm::StormException& Storm::StormException::operator=(const Storm::StormException &other) noexcept
{
	if (this != &other)
	{
		destroyExceptionData(_stackTrace);

		copyExceptionData(other._stackTrace, _stackTrace);
		_stackTraceSize = other._stackTraceSize;
	}

	return *this;
}

const std::string_view Storm::StormException::stackTrace() const noexcept
{
	return std::string_view{ _stackTrace._What, _stackTraceSize };
}
