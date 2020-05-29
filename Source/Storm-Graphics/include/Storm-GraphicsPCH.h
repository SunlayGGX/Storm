#pragma once


#include <d3d11.h>
#include <DirectXMath.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

#include "StormPrerequisite.h"

#include "Logging.h"
#define STORM_MODULE_NAME "Graphics"


#include "ThrowIfFailed.h"


template<class Type> using ComPtr = Microsoft::WRL::ComPtr<Type>;
