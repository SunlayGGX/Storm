#pragma once

#include "StormMacro.h"


int runTestOrDiscovery(int argc, const char* argv[]);


#ifdef STORM_AUTOMATION_AUTO_GENERATE_MAIN_DISCOVERY

int main(int argc, const char* argv[])
{
	return runTestOrDiscovery(argc, argv);
}

#endif

#ifndef STORM_AUTOMATION_NO_AUTO_LIBRARY_LINK
#	pragma comment(lib, STORM_PLUGIN_NAME("StormAutomation-Base"))
#endif
