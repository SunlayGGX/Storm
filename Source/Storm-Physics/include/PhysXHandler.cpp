#include "PhysXHandler.h"

#include "CustomPhysXLogger.h"

#include "ThrowException.h"
#include "Version.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "GeneralSimulationData.h"


namespace
{
	physx::PxDefaultAllocator g_defaultAllocator;
	Storm::CustomPhysXLogger g_physXLogger;
}

Storm::PhysXHandler::PhysXHandler() :
	_foundationInstance{ PxCreateFoundation(PX_PHYSICS_VERSION, g_defaultAllocator, g_physXLogger) }
{
	constexpr Storm::Version currentPhysXVersion{ PX_PHYSICS_VERSION_MAJOR, PX_PHYSICS_VERSION_MINOR, PX_PHYSICS_VERSION_BUGFIX };
	if (currentPhysXVersion != Storm::Version{ 4, 0, 0 })
	{
		LOG_WARNING << "PhysX version mismatches with what we used in our development. What we use currently is PhysX version " << currentPhysXVersion << " and what we used was PhysX version 4.0.0...";
	}

	if (!_foundationInstance)
	{
		Storm::throwException<std::exception>("PhysX foundation couldn't be created!");
	}


	// Create the Physics object.

#if defined(_DEBUG) || defined(DEBUG)
	constexpr bool recordMemoryAlloc = true;
#else
	constexpr bool recordMemoryAlloc = false;
#endif

	physx::PxTolerancesScale toleranceScale;
	_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *_foundationInstance, toleranceScale, recordMemoryAlloc);
	if (!_physics)
	{
		Storm::throwException<std::exception>("PxCreatePhysics failed! We cannot use PhysX features, aborting!");
	}

	_physics->registerDeletionListener(*this, physx::PxDeletionEventFlag::Enum::eUSER_RELEASE);


	// Create the PhysX Scene

	Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

	const Storm::GeneralSimulationData &simulData = configMgr.getGeneralSimulationData();

	physx::PxSceneDesc sceneDesc{ _physics->getTolerancesScale() };
	sceneDesc.gravity = physx::PxVec3{ simulData._gravity.x(), simulData._gravity.y(), simulData._gravity.z() };

	sceneDesc.flags |= physx::PxSceneFlag::eENABLE_PCM;
	sceneDesc.gpuMaxNumPartitions = 8;
	
	sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;

	if (!sceneDesc.cpuDispatcher)
	{
		_cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(1);
		if (!_cpuDispatcher) 
		{
			Storm::throwException<std::exception>("PhysX Cpu dispatcher creation failed!");
		}
		sceneDesc.cpuDispatcher = _cpuDispatcher.get();
	}

	_scene = _physics->createScene(sceneDesc);
	if (!_scene)
	{
		Storm::throwException<std::exception>("PhysX main scene creation failed!");
	}
}

Storm::PhysXHandler::~PhysXHandler()
{
	_physics->unregisterDeletionListener(*this);
}

physx::PxPhysics& Storm::PhysXHandler::getPhysics() const
{
	return *_physics;
}

void Storm::PhysXHandler::onRelease(const physx::PxBase* observed, void* userData, physx::PxDeletionEventFlag::Enum deletionEvent)
{
	PX_UNUSED(userData);
	PX_UNUSED(deletionEvent);

	if (observed->is<physx::PxRigidActor>())
	{
		LOG_DEBUG << "A PhysX actor was just released!";
		// TODO : Do more than this...
	}
}

void Storm::PhysXHandler::setGravity(const Storm::Vector3 &newGravity)
{
	physx::PxVec3 physXGravity{ newGravity.x(), newGravity.y(), newGravity.z() };

	physx::PxSceneWriteLock lock{ *_scene };
	_scene->setGravity(physXGravity);
}
