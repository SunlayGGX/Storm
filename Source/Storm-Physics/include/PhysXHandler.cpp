#include "PhysXHandler.h"

#include "CustomPhysXLogger.h"

#include "ThrowException.h"
#include "Version.h"
#include "PhysXCoordHelpers.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "GeneralSimulationData.h"
#include "RigidBodySceneData.h"
#include "CollisionType.h"


namespace
{
	physx::PxDefaultAllocator g_defaultAllocator;
	Storm::CustomPhysXLogger g_physXLogger;

	Storm::UniquePointer<physx::PxShape> createSphereShape(physx::PxPhysics &physics, const Storm::RigidBodySceneData &rbSceneData, const std::vector<Storm::Vector3> &vertices, physx::PxMaterial* rbMaterial)
	{
		std::mutex mutex;
		float maxDistance = 0.f;

		std::for_each(std::execution::par, std::begin(vertices), std::end(vertices), [&vertices, &mutex, &maxDistance](const Storm::Vector3 &vect)
		{
			float maxDistanceTmp = 0.f;
			float val;
			for (const Storm::Vector3 &vertex : vertices)
			{
				val = (vertex - vect).squaredNorm();
				if (val > maxDistanceTmp)
				{
					maxDistanceTmp = val;
				}
			}

			std::lock_guard<std::mutex> lock{ mutex };
			if (maxDistanceTmp > maxDistance)
			{
				maxDistance = maxDistanceTmp;
			}
		});

		return physics.createShape(physx::PxSphereGeometry{ std::sqrt(maxDistance) / 2.f }, &rbMaterial, 1, true);
	}

	Storm::UniquePointer<physx::PxShape> createBoxShape(physx::PxPhysics &physics, const Storm::RigidBodySceneData &rbSceneData, const std::vector<Storm::Vector3> &vertices, physx::PxMaterial* rbMaterial)
	{
		float maxX = 0.f;
		float maxY = 0.f;
		float maxZ = 0.f;

		const std::size_t verticeCount = vertices.size();
		for (std::size_t iter = 0; iter < verticeCount; ++iter)
		{
			const Storm::Vector3 &vertex = vertices[iter];
			for (std::size_t jiter = iter + 1; jiter < verticeCount; ++jiter)
			{
				Storm::Vector3 diff = vertex - vertices[jiter];
				diff.x() = std::abs(diff.x());
				diff.y() = std::abs(diff.y());
				diff.z() = std::abs(diff.z());

				if (diff.x() > maxX)
				{
					maxX = diff.x();
				}
				if (diff.y() > maxY)
				{
					maxY = diff.y();
				}
				if (diff.z() > maxZ)
				{
					maxZ = diff.z();
				}
			}
		}

		return physics.createShape(physx::PxBoxGeometry{ maxX / 2.f, maxY / 2.f, maxZ / 2.f }, &rbMaterial, 1, true);
	}
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
	sceneDesc.gravity = Storm::convertToPx(simulData._gravity);

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
	const physx::PxVec3 physXGravity = Storm::convertToPx(newGravity);
	LOG_COMMENT << "Gravity set to { x=" << newGravity.x() << ", y=" << newGravity.y() << ", z=" << newGravity.z() << " }";

	physx::PxSceneWriteLock lock{ *_scene };
	_scene->setGravity(physXGravity);
}

void Storm::PhysXHandler::update(std::mutex &fetchingMutex, float deltaTime)
{
	_scene->simulate(deltaTime);

	physx::PxU32 errorCode = 0;
	{
		std::lock_guard<std::mutex> lock{ fetchingMutex };
		_scene->fetchResults(true, &errorCode);
	}

	if (errorCode != 0)
	{
		LOG_ERROR << "Error happened in Physics simulation. Error code was " << errorCode;
	}
}

Storm::UniquePointer<physx::PxRigidStatic> Storm::PhysXHandler::createStaticRigidBody(const Storm::RigidBodySceneData &rbSceneData)
{
	const physx::PxTransform initialPose = Storm::convertToPx(rbSceneData._translation, rbSceneData._rotation);
	
	Storm::UniquePointer<physx::PxRigidStatic> result{ _physics->createRigidStatic(initialPose) };
	_scene->addActor(*result);

	return result;
}

Storm::UniquePointer<physx::PxRigidDynamic> Storm::PhysXHandler::createDynamicRigidBody(const Storm::RigidBodySceneData &rbSceneData)
{
	const physx::PxTransform initialPose = Storm::convertToPx(rbSceneData._translation, rbSceneData._rotation);

	Storm::UniquePointer<physx::PxRigidDynamic> result{ _physics->createRigidDynamic(initialPose) };
	_scene->addActor(*result);

	return result;
}

Storm::UniquePointer<physx::PxMaterial> Storm::PhysXHandler::createRigidBodyMaterial(const Storm::RigidBodySceneData &rbSceneData)
{
	return Storm::UniquePointer<physx::PxMaterial>{ _physics->createMaterial(rbSceneData._staticFrictionCoefficient, rbSceneData._dynamicFrictionCoefficient, rbSceneData._restitutionCoefficient) };
}

Storm::UniquePointer<physx::PxShape> Storm::PhysXHandler::createRigidBodyShape(const Storm::RigidBodySceneData &rbSceneData, const std::vector<Storm::Vector3> &vertices, physx::PxMaterial* rbMaterial)
{
	switch (rbSceneData._collisionShape)
	{
	case Storm::CollisionType::Sphere:
		return createSphereShape(*_physics, rbSceneData, vertices, rbMaterial);

	case Storm::CollisionType::Cube:
		return createBoxShape(*_physics, rbSceneData, vertices, rbMaterial);

	case Storm::CollisionType::None:
	default:
		return Storm::UniquePointer<physx::PxShape>{};
	}
}
