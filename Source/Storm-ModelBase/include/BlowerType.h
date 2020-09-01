#pragma once


#include "BlowerDef.h"

namespace Storm
{
#define STORM_XMACRO_GENERATE_ELEMENTARY_BLOWER(BlowerTypeName, BlowerTypeXmlName, EffectAreaType, MeshMakerType) BlowerTypeName,
	enum class BlowerType
	{
		// The bad value...
		None,

		STORM_XMACRO_GENERATE_BLOWERS_CODE
	};
#undef STORM_XMACRO_GENERATE_ELEMENTARY_BLOWER
}
