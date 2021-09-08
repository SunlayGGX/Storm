#pragma once

#define _USE_MATH_DEFINES

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
#include <bitset>

// Algorithm
#include <algorithm>
#include <execution>

// Container
#include <vector>
#include <map>
#include <span>

// Threading
#include <atomic>
#include <mutex>
#include <thread>

// Because std::string and std::string_view operator+ is really ankward, I'll add those operators to make the concat less strenuous. 
#include "StringOperator.h"

#undef _USE_MATH_DEFINES
