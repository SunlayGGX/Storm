#include "DebuggerHelper.h"

#include "StormMacro.h"

#include "LeanWindowsInclude.h"

#include "VectoredExceptionDisplayMode.h"

#include <boost/stacktrace.hpp>

namespace
{
	static bool g_shouldLogAllVectoredExceptions = true;

	static constexpr std::string_view unknownSeDescription()
	{
		return "UNKNOWN EXCEPTION OR CUSTOM NON SE EXCEPTION";
	}

	static std::string_view seDescription(const unsigned int code)
	{
#define STORM_TRADUCE_SEH_CODE_DESCRIPTION(code) case code: return #code
		switch (code) 
		{
		STORM_TRADUCE_SEH_CODE_DESCRIPTION(EXCEPTION_ACCESS_VIOLATION);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION(EXCEPTION_ARRAY_BOUNDS_EXCEEDED);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION(EXCEPTION_BREAKPOINT);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION(EXCEPTION_DATATYPE_MISALIGNMENT);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION(EXCEPTION_FLT_DENORMAL_OPERAND);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION(EXCEPTION_FLT_DIVIDE_BY_ZERO);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION(EXCEPTION_FLT_INEXACT_RESULT);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION(EXCEPTION_FLT_INVALID_OPERATION);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION(EXCEPTION_FLT_OVERFLOW);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION(EXCEPTION_FLT_STACK_CHECK);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION(EXCEPTION_FLT_UNDERFLOW);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION(EXCEPTION_ILLEGAL_INSTRUCTION);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION(EXCEPTION_GUARD_PAGE);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION(EXCEPTION_IN_PAGE_ERROR);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION(EXCEPTION_INT_DIVIDE_BY_ZERO);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION(EXCEPTION_INT_OVERFLOW);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION(EXCEPTION_INVALID_DISPOSITION);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION(EXCEPTION_NONCONTINUABLE_EXCEPTION);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION(EXCEPTION_PRIV_INSTRUCTION);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION(EXCEPTION_SINGLE_STEP);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION(EXCEPTION_STACK_OVERFLOW);
		default: return unknownSeDescription();
		}
	}
#undef STORM_TRADUCE_SEH_CODE_DESCRIPTION

	enum class GravityAction
	{
		Ignore,
		Low,
		Normal,
		Irreversible,
	};

	GravityAction getDescritpionGravity(const unsigned int code)
	{
#define STORM_TRADUCE_SEH_CODE_DESCRIPTION_GRAVITY(code, irreversibility) case code: return GravityAction::irreversibility
		switch (code)
		{
		STORM_TRADUCE_SEH_CODE_DESCRIPTION_GRAVITY(EXCEPTION_ACCESS_VIOLATION, Irreversible);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION_GRAVITY(EXCEPTION_ARRAY_BOUNDS_EXCEEDED, Normal);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION_GRAVITY(EXCEPTION_BREAKPOINT, Ignore);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION_GRAVITY(EXCEPTION_DATATYPE_MISALIGNMENT, Ignore); // On Windows, it is not so important, will induce performance issue at best.
		STORM_TRADUCE_SEH_CODE_DESCRIPTION_GRAVITY(EXCEPTION_FLT_DENORMAL_OPERAND, Low);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION_GRAVITY(EXCEPTION_FLT_DIVIDE_BY_ZERO, Low);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION_GRAVITY(EXCEPTION_FLT_INEXACT_RESULT, Low);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION_GRAVITY(EXCEPTION_FLT_INVALID_OPERATION, Low);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION_GRAVITY(EXCEPTION_FLT_OVERFLOW, Low);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION_GRAVITY(EXCEPTION_FLT_STACK_CHECK, Low);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION_GRAVITY(EXCEPTION_FLT_UNDERFLOW, Low);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION_GRAVITY(EXCEPTION_ILLEGAL_INSTRUCTION, Irreversible);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION_GRAVITY(EXCEPTION_GUARD_PAGE, Normal); // I don't really understand it, but it seems heavy stuff. In the doubt I set it as normal.
		STORM_TRADUCE_SEH_CODE_DESCRIPTION_GRAVITY(EXCEPTION_IN_PAGE_ERROR, Normal);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION_GRAVITY(EXCEPTION_INT_DIVIDE_BY_ZERO, Low);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION_GRAVITY(EXCEPTION_INT_OVERFLOW, Low);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION_GRAVITY(EXCEPTION_INVALID_DISPOSITION, Normal); // I don't really understand it (and the doc is not helpful). In the doubt I set it as normal.
		STORM_TRADUCE_SEH_CODE_DESCRIPTION_GRAVITY(EXCEPTION_INVALID_HANDLE, Irreversible); // It is like a sigsegv, but with OS handle...
		STORM_TRADUCE_SEH_CODE_DESCRIPTION_GRAVITY(EXCEPTION_NONCONTINUABLE_EXCEPTION, Irreversible);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION_GRAVITY(EXCEPTION_PRIV_INSTRUCTION, Irreversible);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION_GRAVITY(EXCEPTION_SINGLE_STEP, Ignore);
		STORM_TRADUCE_SEH_CODE_DESCRIPTION_GRAVITY(EXCEPTION_STACK_OVERFLOW, Irreversible);
		default: return GravityAction::Normal;
		}
#undef STORM_TRADUCE_SEH_CODE_DESCRIPTION_GRAVITY
	}

	LONG stormVectoredExceptionHandler(_EXCEPTION_POINTERS* exceptionInfo)
	{
		bool isNonContinuable;
		bool isIrreversibleException;
		bool ignore = false;
		std::string_view seDescriptionStr;
		if (exceptionInfo && exceptionInfo->ExceptionRecord)
		{
			seDescriptionStr = seDescription(exceptionInfo->ExceptionRecord->ExceptionCode);
			const GravityAction gravity = getDescritpionGravity(exceptionInfo->ExceptionRecord->ExceptionCode);
			ignore = gravity == GravityAction::Ignore;
			isIrreversibleException = gravity == GravityAction::Irreversible;
			isNonContinuable = exceptionInfo->ExceptionRecord->ExceptionFlags == EXCEPTION_NONCONTINUABLE;
		}
		else
		{
			seDescriptionStr = unknownSeDescription();
			isIrreversibleException = false;
			isNonContinuable = false;
		}

		if (!ignore)
		{
			if (isIrreversibleException)
			{
				// I know this isn't good (we shouldn't allocate or acquire OS resources in a vectored exception handler), but until issues happens, I'll keep it like this.
				// Yuck. After all, even std::cout would acquire something because it is synchronized stdio (even if I think they would be safer, but anyway, I want to see my log so cout won't do for me).
				LOG_FATAL <<
					"Irreversible exception signal (fatal) received: " << seDescriptionStr << ".\n"
					"The stacktrace :\n" << boost::stacktrace::stacktrace() << "\n";

				// Wait a little before terminating.
				std::this_thread::sleep_for(std::chrono::milliseconds{ 1000 });
			}
			else if (g_shouldLogAllVectoredExceptions)
			{
				if (isNonContinuable)
				{
					LOG_ERROR <<
						"Non Continuable exception received: " << seDescriptionStr << ".\n"
						"The stacktrace :\n" << boost::stacktrace::stacktrace() << "\n";
				}
				else
				{
					LOG_ERROR <<
						"Continuable exception received: " << seDescriptionStr << ".\n"
						"The stacktrace :\n" << boost::stacktrace::stacktrace() << "\n";
				}
			}
		}

		// stormVectoredExceptionHandler is just a hook before the real hook to retrieve and print more info about the exception that happened,
		// it doesn't intend to replace other hooks, therefore we forward to real hooks.
		return EXCEPTION_CONTINUE_SEARCH;
	}

	class VectoredExceptionHandlerHooker
	{
	public:
		constexpr VectoredExceptionHandlerHooker() = default;

		void doHooking()
		{
			if (!_active)
			{
				_handler = ::AddVectoredExceptionHandler(TRUE, stormVectoredExceptionHandler);
				if (_handler != NULL)
				{
					_active = true;
					LOG_DEBUG << "Vectored exception handler hooked successfully.";
				}
				else
				{
					_active = false;
					LOG_DEBUG_ERROR << "Vectored exception handler hooking failed. Exception debugging won't be able to print exceptions infos!";
				}
			}
		}

		bool doUnhooking() noexcept
		{
			if (_active)
			{
				// if the return is 0, then we failed to unhook. If we were active then we did hook. Therefore the hook is still there.
				_active = ::RemoveVectoredExceptionHandler(_handler) == 0;
				if (!_active)
				{
					_handler = nullptr;
				}
			}

			return !_active;
		}

		~VectoredExceptionHandlerHooker() noexcept
		{
			this->doUnhooking();
		}

	private:
		bool _active = false;
		PVOID _handler;
	} g_raiiHooker;
}


void Storm::waitForDebuggerToAttach(bool breakAfter /*= false*/)
{
	while (::IsDebuggerPresent() == FALSE)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
	}

	if (breakAfter)
	{
		__debugbreak();
	}
}

void Storm::setupFullAssertionBox()
{
	// assert is useless in release.

#if defined(DEBUG) || defined(_DEBUG)
	::_set_error_mode(_OUT_TO_MSGBOX);
#endif
}

void Storm::unhookVectoredExceptionsDebugging()
{
	g_raiiHooker.doUnhooking();
}

void Storm::setLogVectoredExceptionsDisplayMode(Storm::VectoredExceptionDisplayMode displayMode)
{
	switch (displayMode)
	{
	case Storm::VectoredExceptionDisplayMode::DisplayAll:
		g_shouldLogAllVectoredExceptions = true;
		g_raiiHooker.doHooking();
		break;

	case Storm::VectoredExceptionDisplayMode::DisplayFatal:
		g_shouldLogAllVectoredExceptions = false;
		g_raiiHooker.doHooking();
		break;

	case Storm::VectoredExceptionDisplayMode::None:
		if (!g_raiiHooker.doUnhooking())
		{
			LOG_DEBUG_ERROR << "Failed to unhook vectored exception handler!";
		}
		break;

	default:
		break;
	}
}


// Good to know: we can also traduce SE exception to normal C++ exception using _set_se_translator.
// Maybe I'll do it another day. For now I don't need it and I don't want to have throwing for any C signals happening like 0 division or numbers overflows...
