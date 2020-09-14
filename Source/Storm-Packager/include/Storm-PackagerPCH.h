#pragma once

// C4624: 'XXXX': destructor was implicitly defined as deleted => This is exactly what we want (for static class that shouldn't be instantiated).
#pragma warning(disable: 4624)


#include "StormStdPrerequisite.h"

#define STORM_MODULE_NAME "Packager"
#include "Logging.h"
