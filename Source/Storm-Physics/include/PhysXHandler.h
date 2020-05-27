#pragma once

#include "UniquePointer.h"


namespace Storm
{
	class PhysXHandler
	{
	public:
		PhysXHandler();
		~PhysXHandler();

	private:
		Storm::UniquePointer<physx::PxFoundation> _foundationInstance;
	};
}
