#include "PhysXHandler.h"

#include "CustomPhysXLogger.h"
#include "PhysXDebugger.h"

#include "Version.h"
#include "PhysXCoordHelpers.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"
#include "ITimeManager.h"
#include "IOSManager.h"

#include "SceneSimulationConfig.h"
#include "SceneRigidBodyConfig.h"
#include "SceneConstraintConfig.h"
#include "ScenePhysicsConfig.h"
#include "GeneralApplicationConfig.h"

#include "CollisionType.h"

#define STORM_USE_FAST_SPHERE_SHAPE_ALGO true

#if !STORM_USE_FAST_SPHERE_SHAPE_ALGO
#	include "RunnerHelper.h"
#endif


namespace
{
	physx::PxDefaultAllocator g_defaultAllocator;
	Storm::CustomPhysXLogger g_physXLogger;

	Storm::UniquePointer<physx::PxShape> createSphereShape(physx::PxPhysics &physics, const std::vector<Storm::Vector3> &vertices, physx::PxMaterial* rbMaterial)
	{
#if STORM_USE_FAST_SPHERE_SHAPE_ALGO
		// This algo supposes the sphere is uniform.
		auto oppositeVertexes = std::minmax_element(std::execution::par, std::begin(vertices), std::end(vertices), [](const Storm::Vector3 &vect1, const Storm::Vector3 &vect2)
		{
			return vect1.x() < vect2.x();
		});

		const float maxDistance = std::fabs(oppositeVertexes.first->x() - oppositeVertexes.second->x());
		return physics.createShape(physx::PxSphereGeometry{ maxDistance / 2.f }, &rbMaterial, 1, true);

#else
		// This is the brute force algorithm that always works, but is really slow...

		std::mutex mutex;
		std::atomic<float> maxDistance = 0.f;

		Storm::runParallel(vertices, [&vertices, &mutex, &maxDistance, vertexCount = vertices.size()](const Storm::Vector3 &vect, std::size_t verticeIndex)
		{
			// Skip the current.
			++verticeIndex;

			float maxDistanceTmp = 0.f;
			float val;
			for (; verticeIndex < vertexCount; ++verticeIndex)
			{
				const Storm::Vector3 &vertex = vertices[verticeIndex];
				val = (vertex - vect).squaredNorm();
				if (val > maxDistanceTmp)
				{
					maxDistanceTmp = val;
				}
			}

			if (maxDistanceTmp > maxDistance)
			{
				std::lock_guard<std::mutex> lock{ mutex };
				if (maxDistanceTmp > maxDistance)
				{
					maxDistance = maxDistanceTmp;
				}
			}
		});

		maxDistance = std::sqrtf(maxDistance);
		return physics.createShape(physx::PxSphereGeometry{ maxDistance / 2.f }, &rbMaterial, 1, true);
#endif
	}

	Storm::UniquePointer<physx::PxShape> createBoxShape(physx::PxPhysics &physics, const std::vector<Storm::Vector3> &vertices, physx::PxMaterial* rbMaterial)
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

	Storm::UniquePointer<physx::PxShape> createCustomShape(physx::PxPhysics &physics, const physx::PxCooking &cooking, const Storm::SceneRigidBodyConfig &rbSceneConfig, const std::vector<Storm::Vector3> &vertices, const std::vector<uint32_t> &indexes, physx::PxMaterial* rbMaterial, std::vector<Storm::UniquePointer<physx::PxTriangleMesh>> &inOutRegisteredRef)
	{
		physx::PxTriangleMeshDesc meshDesc;
		meshDesc.points.count = static_cast<physx::PxU32>(vertices.size());
		meshDesc.points.stride = sizeof(Storm::Vector3);
		meshDesc.points.data = vertices.data();

		// We need to invert the clock wise rotation of triangles if the rigid body collision is pointing inside (like in the case of a wall).
		std::vector<uint32_t> indexSwapped;
		if (rbSceneConfig._isWall)
		{
			indexSwapped.resize(indexes.size());
			for (std::size_t index = 0; index < indexes.size(); index += 3)
			{
				indexSwapped[index] = indexes[index];
				indexSwapped[index + 1] = indexes[index + 2];
				indexSwapped[index + 2] = indexes[index + 1];
			}

			meshDesc.triangles.count = static_cast<physx::PxU32>(indexSwapped.size() / 3);
			meshDesc.triangles.stride = 3 * sizeof(uint32_t);
			meshDesc.triangles.data = indexSwapped.data();
		}
		else
		{
			meshDesc.triangles.count = static_cast<physx::PxU32>(indexes.size() / 3);
			meshDesc.triangles.stride = 3 * sizeof(uint32_t);
			meshDesc.triangles.data = indexes.data();
		}

		physx::PxDefaultMemoryOutputStream writeBuffer;
		physx::PxTriangleMeshCookingResult::Enum res = physx::PxTriangleMeshCookingResult::eFAILURE;

		if (!cooking.cookTriangleMesh(meshDesc, writeBuffer, &res))
		{
			LOG_ERROR << "Cannot cook mesh... Res was " << res;
			return nullptr;
		}

		switch (res)
		{
		case physx::PxTriangleMeshCookingResult::eFAILURE:
			LOG_ERROR << "Mesh cooking result resulted in a failure " << res;
			return nullptr;

		case physx::PxTriangleMeshCookingResult::eLARGE_TRIANGLE:
			LOG_WARNING << "Mesh will have large triangle resulting in poor performance (from the PhysX guide)... Beware. " << res;
			break;
		}

		Storm::UniquePointer<physx::PxTriangleMesh> ownedPtr = cooking.createTriangleMesh(meshDesc, physics.getPhysicsInsertionCallback());

		physx::PxTriangleMeshGeometry geometry;
		geometry.triangleMesh = ownedPtr.get();

		if (!geometry.isValid())
		{
			LOG_ERROR << "Triangle mesh geometry isn't valid!";
			return nullptr;
		}

		Storm::UniquePointer<physx::PxShape> result = physics.createShape(geometry, *rbMaterial, true);

		inOutRegisteredRef.emplace_back(std::move(ownedPtr));

		return result;
	}

	physx::PxFilterFlags customCollisionFilterShader(physx::PxFilterObjectAttributes /*attributes0*/, physx::PxFilterData /*filterData0*/, physx::PxFilterObjectAttributes /*attributes1*/, physx::PxFilterData /*filterData1*/, physx::PxPairFlags &retPairFlags, const void* /*constantBlock*/, physx::PxU32 /*constantBlockSize*/)
	{
		retPairFlags =
			physx::PxPairFlag::eCONTACT_DEFAULT |
			physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS |
			physx::PxPairFlag::eNOTIFY_CONTACT_POINTS
			;

		return physx::PxFilterFlag::eNOTIFY;
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
		Storm::throwException<Storm::Exception>("PhysX foundation couldn't be created!");
	}


	// Create the physics debugger network object.
	_physXDebugger = std::make_unique<Storm::PhysXDebugger>(*_foundationInstance);


	// Create the Physics object.

#if defined(_DEBUG) || defined(DEBUG)
	constexpr bool recordMemoryAlloc = true;
#else
	constexpr bool recordMemoryAlloc = false;
#endif

	physx::PxTolerancesScale toleranceScale;
	_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *_foundationInstance, toleranceScale, recordMemoryAlloc, _physXDebugger->getPvd());
	if (!_physics)
	{
		Storm::throwException<Storm::Exception>("PxCreatePhysics failed! We cannot use PhysX features, aborting!");
	}

	_physics->registerDeletionListener(*this, physx::PxDeletionEventFlag::Enum::eUSER_RELEASE);

	// Create the cooking object.
	physx::PxTolerancesScale scale;
	scale.length = 0.00001f;

	physx::PxCookingParams cookingParams{ scale };
	_cooking = PxCreateCooking(PX_PHYSICS_VERSION, *_foundationInstance, cookingParams);
	if (!_cooking)
	{
		Storm::throwException<Storm::Exception>("PxCreateCooking failed! We cannot use PhysX cooking, aborting!");
	}

	// Create the PhysX Scene

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

	const Storm::SceneSimulationConfig &sceneSimulationConfig = configMgr.getSceneSimulationConfig();

	_shouldQuitOnContactWithFloor = !std::isnan(sceneSimulationConfig._exitSimulationFloorLevel);
	_lowestFloorY = sceneSimulationConfig._exitSimulationFloorLevel;

	physx::PxSceneDesc sceneDesc{ _physics->getTolerancesScale() };
	sceneDesc.gravity = Storm::convertToPx(sceneSimulationConfig._gravity);

	const Storm::ScenePhysicsConfig &scenePhysicsConfig = configMgr.getScenePhysicsConfig();
# define STORM_ADD_FLAG_IF_CONFIG(configFlag, flagName) if (scenePhysicsConfig.configFlag) sceneDesc.flags |= physx::PxSceneFlag::flagName

	STORM_ADD_FLAG_IF_CONFIG(_enablePCM, eENABLE_PCM);
	STORM_ADD_FLAG_IF_CONFIG(_enableAdaptiveForce, eADAPTIVE_FORCE);
	STORM_ADD_FLAG_IF_CONFIG(_enableFrictionEveryIteration, eENABLE_FRICTION_EVERY_ITERATION);
	STORM_ADD_FLAG_IF_CONFIG(_enableStabilization, eENABLE_STABILIZATION);
	STORM_ADD_FLAG_IF_CONFIG(_enableKinematicPairs, eENABLE_KINEMATIC_PAIRS);
	STORM_ADD_FLAG_IF_CONFIG(_enableKinematicStaticPairs, eENABLE_KINEMATIC_STATIC_PAIRS);
	STORM_ADD_FLAG_IF_CONFIG(_enableAveragePoint, eENABLE_AVERAGE_POINT);
	STORM_ADD_FLAG_IF_CONFIG(_enableEnhancedDeterminism, eENABLE_ENHANCED_DETERMINISM);
	STORM_ADD_FLAG_IF_CONFIG(_enableCCD, eENABLE_CCD);

#undef STORM_ADD_FLAG_IF_CONFIG

	sceneDesc.gpuMaxNumPartitions = 8;

	if (_shouldQuitOnContactWithFloor)
	{
		sceneDesc.filterShader = customCollisionFilterShader;
	}
	else
	{
		sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
	}
	sceneDesc.simulationEventCallback = this;

	if (!sceneDesc.cpuDispatcher)
	{
		_cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(1);
		if (!_cpuDispatcher) 
		{
			Storm::throwException<Storm::Exception>("PhysX Cpu dispatcher creation failed!");
		}
		sceneDesc.cpuDispatcher = _cpuDispatcher.get();
	}

	_scene = _physics->createScene(sceneDesc);

	if (!_scene)
	{
		Storm::throwException<Storm::Exception>("PhysX main scene creation failed!");
	}

	_physXDebugger->finishSetup(_scene.get());
}

Storm::PhysXHandler::~PhysXHandler()
{
	_triangleMeshReferences.clear();

	_physics->unregisterDeletionListener(*this);

	_physXDebugger->prepareDestroy();

	_scene.reset();
	_cpuDispatcher.reset();
	_physics.reset();
	_physXDebugger.reset();
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

Storm::Vector3 Storm::PhysXHandler::getGravity() const
{
	return Storm::convertToStorm(_scene->getGravity());
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

	_physXDebugger->reconnectIfNeeded();
}

Storm::UniquePointer<physx::PxRigidStatic> Storm::PhysXHandler::createStaticRigidBody(const Storm::SceneRigidBodyConfig &rbSceneConfig)
{
	const physx::PxTransform initialPose = Storm::convertToPx(rbSceneConfig._translation, rbSceneConfig._rotation);
	
	Storm::UniquePointer<physx::PxRigidStatic> result{ _physics->createRigidStatic(initialPose) };
	_scene->addActor(*result);

	return result;
}

Storm::UniquePointer<physx::PxRigidDynamic> Storm::PhysXHandler::createDynamicRigidBody(const Storm::SceneRigidBodyConfig &rbSceneConfig)
{
	const physx::PxTransform initialPose = Storm::convertToPx(rbSceneConfig._translation, rbSceneConfig._rotation);

	Storm::UniquePointer<physx::PxRigidDynamic> result{ _physics->createRigidDynamic(initialPose) };
	result->setMass(rbSceneConfig._mass);

#if false
	result->setLinearDamping(0.5f);
	result->setAngularDamping(PX_MAX_F32);

	result->setStabilizationThreshold(1.f);
#endif
	
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
	const Storm::ScenePhysicsConfig &physXConfig = configMgr.getScenePhysicsConfig();
	if (physXConfig._removeDamping)
	{
		result->setAngularDamping(0.f);
		result->setLinearDamping(0.f);
	}

#if true
	// Normally this is already the case but just in case...
	result->setCMassLocalPose(physx::PxTransform{ physx::PxIDENTITY::PxIdentity });
#endif

	if (_shouldQuitOnContactWithFloor)
	{
		result->setRigidBodyFlag(physx::PxRigidBodyFlag::Enum::eENABLE_CCD, true);
	}

	_scene->addActor(*result);

	return result;
}

Storm::UniquePointer<physx::PxMaterial> Storm::PhysXHandler::createRigidBodyMaterial(const Storm::SceneRigidBodyConfig &rbSceneConfig)
{
	return Storm::UniquePointer<physx::PxMaterial>{ _physics->createMaterial(rbSceneConfig._staticFrictionCoefficient, rbSceneConfig._dynamicFrictionCoefficient, rbSceneConfig._restitutionCoefficient) };
}

Storm::UniquePointer<physx::PxShape> Storm::PhysXHandler::createRigidBodyShape(const Storm::SceneRigidBodyConfig &rbSceneConfig, const std::vector<Storm::Vector3> &vertices, const std::vector<uint32_t> &indexes, physx::PxMaterial* rbMaterial)
{
	Storm::UniquePointer<physx::PxShape> result;

	switch (rbSceneConfig._collisionShape)
	{
	case Storm::CollisionType::IndividualParticle:
	case Storm::CollisionType::Sphere:
		result = createSphereShape(*_physics, vertices, rbMaterial);
		break;

	case Storm::CollisionType::Cube:
		result = createBoxShape(*_physics, vertices, rbMaterial);
		break;

	case Storm::CollisionType::Custom:
		if (indexes.empty())
		{
			Storm::throwException<Storm::Exception>("To create a custom shape, we need indexes set (like creating a custom mesh)!");
		}
		result = createCustomShape(*_physics, *_cooking, rbSceneConfig, vertices, indexes, rbMaterial, _triangleMeshReferences);
		break;

	case Storm::CollisionType::None:
	default:
		return Storm::UniquePointer<physx::PxShape>{};
	}

	if (_shouldQuitOnContactWithFloor)
	{
		result->setFlag(physx::PxShapeFlag::Enum::eSIMULATION_SHAPE, true);
	}

	return result;
}

Storm::UniquePointer<physx::PxJoint> Storm::PhysXHandler::createDistanceJoint(const Storm::SceneConstraintConfig &constraintConfig, physx::PxRigidActor* actor1, physx::PxRigidActor* actor2)
{
	physx::PxTransform actor1LinkTransformFrame = physx::PxTransform{ physx::PxIDENTITY::PxIdentity };
	actor1LinkTransformFrame.p += Storm::convertToPx(constraintConfig._rigidBody1LinkTranslationOffset);

	physx::PxTransform actor2LinkTransformFrame = physx::PxTransform{ physx::PxIDENTITY::PxIdentity };
	actor2LinkTransformFrame.p += Storm::convertToPx(constraintConfig._rigidBody2LinkTranslationOffset);

	physx::PxDistanceJoint* tmp = physx::PxDistanceJointCreate(*_physics,
		actor1, actor1LinkTransformFrame,
		actor2, actor2LinkTransformFrame
	);

	Storm::UniquePointer<physx::PxJoint> result{ tmp };

	tmp->setConstraintFlag(physx::PxConstraintFlag::Enum::eCOLLISION_ENABLED, true);
	tmp->setConstraintFlag(physx::PxConstraintFlag::Enum::eENABLE_EXTENDED_LIMITS, false);

	const float maxDistance = constraintConfig._constraintsLength;

	tmp->setDistanceJointFlag(physx::PxDistanceJointFlag::eSPRING_ENABLED, false);

	tmp->setMaxDistance(maxDistance);
	tmp->setTolerance(0.f);
	tmp->setStiffness(PX_MAX_F32);
	tmp->setDamping(PX_MAX_F32);
	tmp->setDistanceJointFlag(physx::PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);
	tmp->setDistanceJointFlag(physx::PxDistanceJointFlag::eMIN_DISTANCE_ENABLED, false);

	return result;
}

std::pair<Storm::UniquePointer<physx::PxJoint>, Storm::UniquePointer<physx::PxJoint>> Storm::PhysXHandler::createSpinnableJoint(const Storm::SceneConstraintConfig &/*constraintConfig*/, physx::PxRigidActor* /*actor1*/, physx::PxRigidActor* /*actor2*/)
{
	// TODO
	std::pair<Storm::UniquePointer<physx::PxJoint>, Storm::UniquePointer<physx::PxJoint>> result;
	return result;
}

void Storm::PhysXHandler::reconnectPhysicsDebugger()
{
	_physXDebugger->reconnectIfNeeded();
}

bool Storm::PhysXHandler::shouldExitIfHitFloor() const
{
	return _shouldQuitOnContactWithFloor;
}

void Storm::PhysXHandler::onConstraintBreak(physx::PxConstraintInfo* /*constraints*/, physx::PxU32 /*count*/)
{

}

void Storm::PhysXHandler::onContact(const physx::PxContactPairHeader &pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
{
	if (_shouldQuitOnContactWithFloor)
	{
		std::vector<physx::PxContactPairPoint> contacts;

		for (physx::PxU32 iter = 0; iter < nbPairs; ++iter)
		{
			const physx::PxContactPair &currentPair = pairs[iter];

			if (currentPair.events & physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
			{
				const physx::PxRigidDynamic* rb;

				if (pairHeader.actors[0]->getConcreteType() == physx::PxConcreteType::Enum::eRIGID_DYNAMIC)
				{
					rb = static_cast<physx::PxRigidDynamic*>(pairHeader.actors[0]);
				}
				else if (pairHeader.actors[1]->getConcreteType() == physx::PxConcreteType::Enum::eRIGID_DYNAMIC)
				{
					rb = static_cast<physx::PxRigidDynamic*>(pairHeader.actors[1]);
				}
				else
				{
					continue;
				}

				const physx::PxRigidStatic* floor;
				if (pairHeader.actors[0]->getConcreteType() == physx::PxConcreteType::Enum::eRIGID_STATIC)
				{
					floor = static_cast<physx::PxRigidStatic*>(pairHeader.actors[0]);
				}
				else if (pairHeader.actors[1]->getConcreteType() == physx::PxConcreteType::Enum::eRIGID_STATIC)
				{
					floor = static_cast<physx::PxRigidStatic*>(pairHeader.actors[1]);
				}
				else
				{
					continue;
				}

				// If we have a contact between a static and a dynamic body (the floor would always be static)
				if (rb && floor)
				{
					contacts.resize(currentPair.contactCount);
					contacts.resize(currentPair.extractContacts(contacts.data(), currentPair.contactCount));

					if (std::find_if(std::begin(contacts), std::end(contacts), [this](const physx::PxContactPairPoint &contact)
					{
						return std::fabs(Storm::convertToStorm(contact.position).y() - _lowestFloorY) < 0.0001f;
					}) != std::end(contacts))
					{
						LOG_COMMENT <<
							"A contact with the floor was triggered.\n" <<
							rb->getName() << " hit the lowest level of " << floor->getName() << ".\n"
							"Therefore, as requested in the global configuration, we'll exit the simulation because it is deemed useless to continue."
							;

						const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

						singletonHolder.getSingleton<Storm::ITimeManager>().quit();

						const Storm::GeneralApplicationConfig &generalAppConfig = singletonHolder.getSingleton<Storm::IConfigManager>().getGeneralApplicationConfig();
						if (generalAppConfig._bipSoundOnFinish)
						{
							const Storm::IOSManager &osMgr = singletonHolder.getSingleton<Storm::IOSManager>();
							osMgr.makeBipSound(std::chrono::milliseconds{ 500 });
						}
					}
				}
			}
		}
	}
}

void Storm::PhysXHandler::onWake(physx::PxActor** /*actors*/, physx::PxU32 /*count*/)
{

}

void Storm::PhysXHandler::onSleep(physx::PxActor** /*actors*/, physx::PxU32 /*count*/)
{

}

void Storm::PhysXHandler::onTrigger(physx::PxTriggerPair* /*pairs*/, physx::PxU32 /*count*/)
{

}

void Storm::PhysXHandler::onAdvance(const physx::PxRigidBody*const* /*bodyBuffer*/, const physx::PxTransform* /*poseBuffer*/, const physx::PxU32 /*count*/)
{

}
