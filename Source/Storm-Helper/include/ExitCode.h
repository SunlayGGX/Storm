#pragma once


namespace Storm
{
	enum class ExitCode : int
	{
		k_success = 0,

		k_stdException = -99,
		k_unknownException = -100,
	};
}
