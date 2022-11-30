#pragma once
#include "BitField.h"


namespace StormExporter
{
	enum class ExportMode
	{
		None = Storm::BitField<false, false>::value,
		Fluid = Storm::BitField<true, false>::value,
		RigidBody = Storm::BitField<false, true>::value,
	};
}
