#pragma once

#include "BitField.h"


namespace Storm
{
	enum class RaycastEnablingFlag : uint8_t
	{
		Disabled =				Storm::BitField<false, false>::value,
		DynamicRigidBodies =	Storm::BitField<false, true>::value,
		Fluids =				Storm::BitField<true, false>::value,
	};
}
