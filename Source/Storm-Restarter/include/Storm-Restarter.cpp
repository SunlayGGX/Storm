// Storm-Packager.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "ExitCode.h"

#include "LeanWindowsInclude.h"

#include <iostream>

#include <boost/process/async_system.hpp>
#include <boost/asio/io_service.hpp>


int main(int argc, const char*const argv[]) try
{
	if (argc < 3)
	{
		Storm::throwException<Storm::Exception>("We must have at least the scene file and the current storm path to start as argument!");
	}

	std::string cmdRebuild = "";

	bool hasStormPath = false;

	constexpr std::string_view stormPathArgToken = "--stormPath=";

	std::string cmd;
	for (int iter = 1; iter < argc; ++iter)
	{
		const std::string_view arg = argv[iter];
		if (arg.find(stormPathArgToken) != std::string_view::npos)
		{
			const std::string_view stormPathArg = arg.substr(stormPathArgToken.size());

			if (stormPathArg.empty())
			{
				Storm::throwException<Storm::Exception>("Storm exe path shouldn't be empty!");
			}
			else if (hasStormPath)
			{
				Storm::throwException<Storm::Exception>("Cannot set storm exe path twice!");
			}
			else
			{
				if (stormPathArg[0] != '"')
				{
					cmdRebuild += '"';
				}
				cmdRebuild += stormPathArg;
				if (stormPathArg.back() != '"')
				{
					cmdRebuild += '"';
				}
				cmdRebuild += ' ';

				hasStormPath = true;
			}
		}
		else
		{
			cmd += arg;
			cmd += ' ';
		}
	}

	cmdRebuild += cmd;

	/*boost::process::child restartStormProc{ cmdRebuild };
	if (!restartStormProc.running())
	{
		Storm::throwException<Storm::Exception>("We couldn't restart storm process with those arguments : " + cmdRebuild);
	}

	restartStormProc.detach();
	return 0;*/

	boost::asio::io_service ioService;
	boost::process::async_system(ioService, [](boost::system::error_code, int) {}, cmdRebuild);

	return 0;
}
catch (const Storm::Exception &ex)
{
	std::cerr <<
		"Unhandled storm exception happened!\n"
		"Message was " << ex.what() << ".\n" << ex.stackTrace()
		;
	return static_cast<int>(Storm::ExitCode::k_stdException);
}
catch (const std::exception &ex)
{
	std::cerr << "Unhandled std exception happened! Message was " << ex.what();
	return static_cast<int>(Storm::ExitCode::k_stdException);
}
catch (...)
{
	std::cerr << "Unhandled unknown exception happened!";
	return static_cast<int>(Storm::ExitCode::k_unknownException);
}
