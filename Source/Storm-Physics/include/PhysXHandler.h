#pragma once

#include "UniquePointer.h"


namespace Storm
{
	struct SceneRigidBodyConfig;
	struct SceneConstraintConfig;

	class PhysXDebugger;

	class PhysXHandler : public physx::PxDeletionListener
	{
	public:
		PhysXHandler();
		~PhysXHandler();

	public:
		physx::PxPhysics& getPhysics() const;

	public:
		void onRelease(const physx::PxBase* observed, void* userData, physx::PxDeletionEventFlag::Enum deletionEvent) final override;

	public:
		void setGravity(const Storm::Vector3 &newGravity);
		Storm::Vector3 getGravity() const;

	public:
		void update(std::mutex &fetchingMutex, float deltaTime);

	public:
		Storm::UniquePointer<physx::PxRigidStatic> createStaticRigidBody(const Storm::SceneRigidBodyConfig &rbSceneConfig);
		Storm::UniquePointer<physx::PxRigidDynamic> createDynamicRigidBody(const Storm::SceneRigidBodyConfig &rbSceneConfig);
		
		Storm::UniquePointer<physx::PxMaterial> createRigidBodyMaterial(const Storm::SceneRigidBodyConfig &rbSceneConfig);
		Storm::UniquePointer<physx::PxShape> createRigidBodyShape(const Storm::SceneRigidBodyConfig &rbSceneConfig, const std::vector<Storm::Vector3> &vertices, const std::vector<uint32_t> &indexes, physx::PxMaterial* rbMaterial);

		Storm::UniquePointer<physx::PxJoint> createDistanceJoint(const Storm::SceneConstraintConfig &constraintConfig, physx::PxRigidActor* actor1, physx::PxRigidActor* actor2);
		std::pair<Storm::UniquePointer<physx::PxJoint>, Storm::UniquePointer<physx::PxJoint>> createSpinnableJoint(const Storm::SceneConstraintConfig &constraintConfig, physx::PxRigidActor* actor1, physx::PxRigidActor* actor2);

	public:
		void reconnectPhysicsDebugger();

	private:
		Storm::UniquePointer<physx::PxFoundation> _foundationInstance;
		Storm::UniquePointer<physx::PxPhysics> _physics;
		Storm::UniquePointer<physx::PxCooking> _cooking;
		Storm::UniquePointer<physx::PxDefaultCpuDispatcher> _cpuDispatcher;
		Storm::UniquePointer<physx::PxScene> _scene;

		std::unique_ptr<Storm::PhysXDebugger> _physXDebugger;

		// Since PhysX doesn't own it.
		std::vector<Storm::UniquePointer<physx::PxTriangleMesh>> _triangleMeshReferences;
	};
}
