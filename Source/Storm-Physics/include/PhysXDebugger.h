#pragma once

#include "UniquePointer.h"


namespace Storm
{
	class PhysXDebugger
	{
	public:
		PhysXDebugger(physx::PxFoundation &foundation);
		~PhysXDebugger();

	public:
		physx::PxPvd* getPvd() const;

	private:
		bool _enabled;
		Storm::UniquePointer<physx::PxPvd> _pvd;
		Storm::UniquePointer<physx::PxPvdTransport> _transport;
	};
}
