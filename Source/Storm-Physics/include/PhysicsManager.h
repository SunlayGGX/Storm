#pragma once


#include "Singleton.h"
#include "IPhysicsManager.h"


namespace Storm
{
	class PhysXHandler;

	class PhysicsDynamicRigidBody;
	class PhysicsStaticsRigidBody;
	
	class PhysicsConstraint;

	class PhysicsManager :
		private Storm::Singleton<PhysicsManager>,
		public Storm::IPhysicsManager
	{
		STORM_DECLARE_SINGLETON(PhysicsManager);

	private:
		void initialize_Implementation();
		void cleanUp_Implementation();

	public:
		void update(float deltaTime) final override;

	public:
		void addPhysicalBody(const Storm::RigidBodySceneData &rbSceneData, const std::vector<Storm::Vector3> &vertexes) final override;
		void bindParentRbToPhysicalBody(const Storm::RigidBodySceneData &rbSceneData, const std::shared_ptr<Storm::IRigidBody> &parentRb) const final override;
		void getMeshTransform(unsigned int meshId, Storm::Vector3 &outTrans, Storm::Vector3 &outRot) const final override;
		void getMeshTransform(unsigned int meshId, Storm::Vector3 &outTrans, Storm::Quaternion &outQuatRot) const final override;
		void applyLocalForces(unsigned int particleSystemId, const std::vector<Storm::Vector3> &position, const std::vector<Storm::Vector3> &force) final override;

		void addConstraint(const Storm::ConstraintData &constraintData) final override;
		void loadConstraints(const std::vector<Storm::ConstraintData> &constraintsToLoad) final override;

	public:
		const Storm::PhysXHandler& getPhysXHandler() const;
		Storm::PhysXHandler& getPhysXHandler();

	public:
		mutable std::mutex _simulationMutex;

	private:
		std::unique_ptr<Storm::PhysXHandler> _physXHandler;

		std::map<unsigned int, std::unique_ptr<Storm::PhysicsStaticsRigidBody>> _staticsRbMap;
		std::map<unsigned int, std::unique_ptr<Storm::PhysicsDynamicRigidBody>> _dynamicsRbMap;

		std::vector<std::unique_ptr<Storm::PhysicsConstraint>> _constraints;
	};
}
