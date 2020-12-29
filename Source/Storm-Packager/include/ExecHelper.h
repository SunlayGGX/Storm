#pragma once

#include "NonInstanciable.h"


namespace StormPackager
{
	struct ExecResult
	{
	public:
		bool _success;
		int _commandExitCode;
		std::string _output;
		std::string _error;
	};

	class ExecHelper : private Storm::NonInstanciable
	{
	public:
		static StormPackager::ExecResult execute(const std::string &command);

		// Git related commands
		static StormPackager::ExecResult checkout(const std::string &checkoutPoint);
		static StormPackager::ExecResult pull();
		static StormPackager::ExecResult stash();
		static StormPackager::ExecResult stashPop();

		static StormPackager::ExecResult getHEADHash();
		static StormPackager::ExecResult getCurrentBranchName();
	};
}
