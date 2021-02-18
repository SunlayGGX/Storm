#pragma once


#include "Singleton.h"
#include "IPhysicsManager.h"
#include "DeclareScriptableItem.h"


namespace Storm
{
	class PhysXHandler;

	class PhysicsDynamicRigidBody;
	class PhysicsStaticsRigidBody;
	
	class PhysicsConstraint;

	class PhysicsManager final :
		private Storm::Singleton<Storm::PhysicsManager>,
		public Storm::IPhysicsManager
	{
		STORM_DECLARE_SINGLETON(PhysicsManager);
		STORM_IS_SCRIPTABLE_ITEM;

	private:
		void initialize_Implementation();
		void cleanUp_Implementation();

	public:
		void notifyIterationStart() final override;
		void update(const float currentTime, float deltaTime) final override;

	public:
		void addPhysicalBody(const Storm::SceneRigidBodyConfig &rbSceneConfig, const std::vector<Storm::Vector3> &vertexes, const std::vector<uint32_t> &indexes) final override;
		void bindParentRbToPhysicalBody(const bool isStatic, const unsigned int rbId, const std::shared_ptr<Storm::IRigidBody> &parentRb) const final override;
		void bindParentRbToPhysicalBody(const Storm::SceneRigidBodyConfig &rbSceneConfig, const std::shared_ptr<Storm::IRigidBody> &parentRb) const final override;
		void getMeshTransform(unsigned int meshId, Storm::Vector3 &outTrans, Storm::Rotation &outRot) const final override;
		void getMeshTransform(unsigned int meshId, Storm::Vector3 &outTrans, Storm::Quaternion &outQuatRot) const final override;
		void applyLocalForces(unsigned int particleSystemId, const std::vector<Storm::Vector3> &position, const std::vector<Storm::Vector3> &force) final override;

		void addConstraint(const Storm::SceneConstraintConfig &constraintConfig) final override;
		void loadConstraints(const std::vector<Storm::SceneConstraintConfig> &constraintsToLoad) final override;

		void loadRecordedConstraint(const Storm::SerializeConstraintLayout &constraintsRecordLayout) final override;

		void getConstraintsRecordLayoutData(std::vector<Storm::SerializeConstraintLayout> &outConstraintsRecordLayout) const final override;
		void getConstraintsRecordFrameData(std::vector<Storm::SerializeRecordContraintsData> &outConstraintsRecordFrameData) const final override;
		void pushConstraintsRecordedFrame(const std::vector<Storm::SerializeRecordContraintsData> &constraintsRecordFrameData) final override;

		Storm::Vector3 getPhysicalForceOnPhysicalBody(const unsigned int id) const final override;
		Storm::Vector3 getForceOnPhysicalBody(const unsigned int id, const float deltaTimeInSecond) const final override;

		void freeFromAnimation(const unsigned int rbId) final override;

		void reconnectPhysicsDebugger() final override;

	public:
		void setRigidBodyAngularDamping(const unsigned int rbId, const float angularVelocityDamping);
		void fixDynamicRigidBodyTranslation(const unsigned int rbId, const bool fixed);

	private:
		void pushPhysicsVisualizationData() const;
		void pushConstraintsVisualizationData() const;

	public:
		const Storm::PhysXHandler& getPhysXHandler() const;
		Storm::PhysXHandler& getPhysXHandler();

	public:
		mutable std::mutex _simulationMutex;

	private:
		std::unique_ptr<Storm::PhysXHandler> _physXHandler;

		std::map<unsigned int, std::unique_ptr<Storm::PhysicsDynamicRigidBody>> _dynamicsRbMap;
		std::map<unsigned int, std::unique_ptr<Storm::PhysicsStaticsRigidBody>> _staticsRbMap;

		std::vector<std::shared_ptr<Storm::PhysicsConstraint>> _constraints;

		bool _rigidBodiesFixated;
	};
}
