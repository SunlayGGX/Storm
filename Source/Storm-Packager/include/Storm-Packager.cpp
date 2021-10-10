// Storm-Packager.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "Application.h"

#include "ExitCode.h"

#include <iostream>


int main(const int argc, const char*const argv[]) try
{
	return static_cast<int>(StormPackager::Application{ argc, argv }.run());
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
