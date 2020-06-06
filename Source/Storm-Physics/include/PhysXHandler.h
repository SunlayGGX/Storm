#pragma once

#include "UniquePointer.h"


namespace Storm
{
	struct RigidBodySceneData;

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

	public:
		void update(std::mutex &fetchingMutex, float deltaTime);

	public:
		Storm::UniquePointer<physx::PxRigidStatic> createStaticRigidBody(const Storm::RigidBodySceneData &rbSceneData);
		Storm::UniquePointer<physx::PxRigidDynamic> createDynamicRigidBody(const Storm::RigidBodySceneData &rbSceneData);
		
		Storm::UniquePointer<physx::PxMaterial> createRigidBodyMaterial(const Storm::RigidBodySceneData &rbSceneData);
		Storm::UniquePointer<physx::PxShape> createRigidBodyShape(const Storm::RigidBodySceneData &rbSceneData, const std::vector<Storm::Vector3> &vertices, physx::PxMaterial* rbMaterial);


	private:
		Storm::UniquePointer<physx::PxFoundation> _foundationInstance;
		Storm::UniquePointer<physx::PxPhysics> _physics;
		Storm::UniquePointer<physx::PxDefaultCpuDispatcher> _cpuDispatcher;
		Storm::UniquePointer<physx::PxScene> _scene;
	};
}
