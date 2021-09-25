#include "ExecHelper.h"

#include "LeanWindowsInclude.h"

#pragma warning(push)
#pragma warning(disable : 4244) // 'argument': conversion from 'ptrdiff_t' to 'int', possible loss of data. Sadly, boost ignored the issue.
#	include <boost\process\system.hpp>
#	include <boost\process\io.hpp>
#pragma warning(pop)

#include <boost\algorithm\string\trim.hpp>

namespace
{
	template<bool trimEnding, class StreamType, class StringType>
	void transfert(StreamType &stream, StringType &outStr)
	{
		enum : std::size_t { k_bufferSize = 1024 };
		outStr.reserve(k_bufferSize);

		std::copy(std::istreambuf_iterator<char>{ stream }, std::istreambuf_iterator<char>{}, std::back_inserter(outStr));

		if constexpr (trimEnding)
		{
			boost::algorithm::trim_if(outStr, boost::is_any_of("\r\n \t"));
		}
	}
}


StormPackager::ExecResult StormPackager::ExecHelper::execute(const std::string &command)
{
	StormPackager::ExecResult result;

	boost::process::ipstream stdOutStream;
	boost::process::ipstream stdErrStream;
	result._commandExitCode = boost::process::system(command, boost::process::std_out > stdOutStream, boost::process::std_err > stdErrStream);

	result._success = result._commandExitCode == 0;

	transfert<true>(stdOutStream, result._output);
	transfert<true>(stdErrStream, result._error);

	return result;
}

StormPackager::ExecResult StormPackager::ExecHelper::checkout(const std::string &checkoutPoint)
{
	if (!checkoutPoint.empty())
	{
		const std::string fullCommand = "git checkout " + checkoutPoint;
		return StormPackager::ExecHelper::execute(fullCommand);
	}
	else
	{
		return StormPackager::ExecResult{
			._success = true,
			._commandExitCode = 0
		};
	}
}

StormPackager::ExecResult StormPackager::ExecHelper::pull()
{
	return StormPackager::ExecHelper::execute("git pull --rebase");
}

StormPackager::ExecResult StormPackager::ExecHelper::stash()
{
	return StormPackager::ExecHelper::execute("git stash");
}

StormPackager::ExecResult StormPackager::ExecHelper::stashPop()
{
	return StormPackager::ExecHelper::execute("git stash pop");
}

StormPackager::ExecResult StormPackager::ExecHelper::getHEADHash()
{
	return StormPackager::ExecHelper::execute("git rev-parse HEAD");
}

StormPackager::ExecResult StormPackager::ExecHelper::getCurrentBranchName()
{
	return StormPackager::ExecHelper::execute("git branch --show-current");
}
