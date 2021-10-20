#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	struct SceneRigidBodyConfig;
	struct SceneConstraintConfig;
	class IRigidBody;

	struct SerializeConstraintLayout;
	struct SerializeRecordContraintsData;

	using OnRigidbodyFixedDelegate = std::function<void(bool)>;

	class IPhysicsManager : public Storm::ISingletonHeldInterface<IPhysicsManager>
	{
	public:
		virtual ~IPhysicsManager() = default;

	public:
		virtual void notifyIterationStart() = 0;
		virtual void update(const float currentTime, float deltaTime) = 0;

	public:
		virtual void addPhysicalBody(const Storm::SceneRigidBodyConfig &rbSceneConfig, const std::vector<Storm::Vector3> &vertexes, const std::vector<uint32_t> &indexes) = 0;
		virtual void bindParentRbToPhysicalBody(const Storm::SceneRigidBodyConfig &rbSceneConfig, const std::shared_ptr<Storm::IRigidBody> &parentRb) const = 0;
		virtual void bindParentRbToPhysicalBody(const bool isStatic, const unsigned int rbId, const std::shared_ptr<Storm::IRigidBody> &parentRb) const = 0;
		virtual void getMeshTransform(unsigned int meshId, Storm::Vector3 &outTrans, Storm::Rotation &outRot) const = 0;
		virtual void getMeshTransform(unsigned int meshId, Storm::Vector3 &outTrans, Storm::Quaternion &outQuatRot) const = 0;
		virtual void applyLocalForces(unsigned int particleSystemId, const std::vector<Storm::Vector3> &position, const std::vector<Storm::Vector3> &force) = 0;

		virtual void loadConstraints(const std::vector<Storm::SceneConstraintConfig> &constraintsToLoad) = 0;
		virtual void addConstraint(const Storm::SceneConstraintConfig &constraintConfig) = 0;

		virtual void loadRecordedConstraint(const Storm::SerializeConstraintLayout &constraintsRecordLayout) = 0;

		virtual void getConstraintsRecordLayoutData(std::vector<Storm::SerializeConstraintLayout> &outConstraintsRecordLayout) const = 0;
		virtual void getConstraintsRecordFrameData(std::vector<Storm::SerializeRecordContraintsData> &outConstraintsRecordFrameData) const = 0;
		virtual void pushConstraintsRecordedFrame(const std::vector<Storm::SerializeRecordContraintsData> &constraintsRecordFrameData) = 0;

		// Doesn't account for fluid forces. Only forces from Physics engine.
		virtual Storm::Vector3 getPhysicalForceOnPhysicalBody(const unsigned int id) const = 0;
		virtual Storm::Vector3 getForceOnPhysicalBody(const unsigned int id, const float deltaTimeInSecond) const = 0;

		virtual Storm::Vector3 getPhysicalBodyCurrentLinearVelocity(const unsigned int id) const = 0;

		virtual void freeFromAnimation(const unsigned int rbId) = 0;

		virtual void reconnectPhysicsDebugger() = 0;

		virtual void setRigidBodiesFixed(const bool shouldFix) = 0;

		// Listen to rigid body fixed callback.
		virtual unsigned short bindOnRigidbodyFixedCallback(Storm::OnRigidbodyFixedDelegate &&callback) = 0;
		virtual void unbindOnRigidbodyFixedCallback(unsigned short callbackId) = 0;
	};
}
