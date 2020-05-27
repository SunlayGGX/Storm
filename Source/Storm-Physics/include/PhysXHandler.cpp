#include "PhysXHandler.h"

#include "CustomPhysXLogger.h"

#include "ThrowException.h"
#include "Version.h"


namespace
{
	physx::PxDefaultAllocator g_defaultAllocator;
	Storm::CustomPhysXLogger g_physXLogger;
}

Storm::PhysXHandler::PhysXHandler() :
	_foundationInstance{ PxCreateFoundation(PX_PHYSICS_VERSION, g_defaultAllocator, g_physXLogger) }
{
	if (!_foundationInstance)
	{
		Storm::throwException<std::exception>("PhysX foundation couldn't be created!");
	}

	constexpr Storm::Version currentPhysXVersion{ PX_PHYSICS_VERSION_MAJOR, PX_PHYSICS_VERSION_MINOR, PX_PHYSICS_VERSION_BUGFIX };
	if (currentPhysXVersion != Storm::Version{ 4, 0, 0 })
	{
		LOG_WARNING << "PhysX version mismatches with what we used in our development. What we use currently is PhysX version " << currentPhysXVersion << " and what we used was PhysX version 4.0.0...";
	}
}

Storm::PhysXHandler::~PhysXHandler()
{

}
