#pragma once


#include "StormStdPrerequisite.h"

#include <catch2/catch.hpp>

// 4624 : 'XXXX': destructor was implicitly defined as deleted => I did it on purpose!... It is to make non instanciable class (purely abstract class).
#define STORM_DISABLED_WARNINGS 4624
#pragma warning(disable: STORM_DISABLED_WARNINGS)
