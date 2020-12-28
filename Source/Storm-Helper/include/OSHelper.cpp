#include "OSHelper.h"

#include "LeanWindowsInclude.h"


std::string Storm::OSHelper::getRawQuotedCommandline()
{
	return Storm::toStdString(::GetCommandLine());
}

