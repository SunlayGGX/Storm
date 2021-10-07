#pragma once


namespace Storm
{
	enum class ExitCode : int
	{
		k_success = 0,
		k_failure = 1,

		k_termination = 100,

		k_safetyTermination = 500,

		k_otherThreadTermination = 200,

		k_stdException = -99,
		k_unknownException = -100,

		k_imbricatedStdException = -101,
		k_imbricatedUnknownException = -102,
	};
}
