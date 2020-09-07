#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	struct RigidBodySceneData;
	struct ConstraintData;
	class IRigidBody;

	class IPhysicsManager : public Storm::ISingletonHeldInterface<IPhysicsManager>
	{
	public:
		virtual ~IPhysicsManager() = default;

	public:
		virtual void update(float deltaTime) = 0;

	public:
		virtual void addPhysicalBody(const Storm::RigidBodySceneData &rbSceneData, const std::vector<Storm::Vector3> &vertexes, const std::vector<uint32_t> &indexes) = 0;
		virtual void bindParentRbToPhysicalBody(const Storm::RigidBodySceneData &rbSceneData, const std::shared_ptr<Storm::IRigidBody> &parentRb) const = 0;
		virtual void bindParentRbToPhysicalBody(const bool isStatic, const unsigned int rbId, const std::shared_ptr<Storm::IRigidBody> &parentRb) const = 0;
		virtual void getMeshTransform(unsigned int meshId, Storm::Vector3 &outTrans, Storm::Vector3 &outRot) const = 0;
		virtual void getMeshTransform(unsigned int meshId, Storm::Vector3 &outTrans, Storm::Quaternion &outQuatRot) const = 0;
		virtual void applyLocalForces(unsigned int particleSystemId, const std::vector<Storm::Vector3> &position, const std::vector<Storm::Vector3> &force) = 0;

		virtual void loadConstraints(const std::vector<Storm::ConstraintData> &constraintsToLoad) = 0;
		virtual void addConstraint(const Storm::ConstraintData &constraintData) = 0;
	};
}
