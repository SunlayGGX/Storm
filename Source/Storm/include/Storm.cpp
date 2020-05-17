#include "Application.h"


int main(int argc, const char* argv[]) try
{
    return static_cast<int>(Storm::Application{ argc, argv }.run());
}
catch (const std::exception &/*ex*/)
{
    return static_cast<int>(Storm::ExitCode::k_stdException);
}
catch (...)
{
    return static_cast<int>(Storm::ExitCode::k_unknownException);
}
