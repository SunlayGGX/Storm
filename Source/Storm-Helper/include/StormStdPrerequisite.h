#pragma once

// Base
#include <memory>
#include <string>
#include <type_traits>
#include <exception>
#include <cassert>
#include <sstream>
#include <chrono>
#include <functional>
#include <filesystem>

// Container
#include <vector>
#include <map>

// Threading
#include <atomic>
#include <mutex>
#include <thread>


// 4624: 'XXXXXX': destructor was implicitly defined as deleted => Yes it is intended for classes that shouldn't be instantiated (abstract classes). NonInstanciable would always fire this warning so we need to disable it.
#define STORM_DISABLE_WARNING_ALL 4624
#pragma warning(disable: STORM_DISABLE_WARNING_ALL)


#include "StormMacro.h"
