#include "Application.h"

#include <iostream>


int main(int argc, const char* argv[]) try
{
	return static_cast<int>(Storm::Application{ argc, argv }.run());
}
catch (const std::exception &ex)
{
	std::cerr << "Fatal error (std::exception received) : " << ex.what() << '\n';
	return static_cast<int>(Storm::ExitCode::k_stdException);
}
catch (...)
{
	std::cerr << "Fatal error (unknown exception received)\n";
	return static_cast<int>(Storm::ExitCode::k_unknownException);
}
