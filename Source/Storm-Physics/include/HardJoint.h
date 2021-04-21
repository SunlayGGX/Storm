#pragma once


namespace physx
{
	class PxRigidActor;
}

namespace Storm
{
	struct SceneConstraintConfig;

	// This one is not from Physics, so does not inherit from PhysXJointBase
	class HardJoint
	{
	public:
		HardJoint(const Storm::SceneConstraintConfig &data, physx::PxRigidActor* actor1, physx::PxRigidActor* actor2);

	public:
		void getJointPositionToArray(Storm::Vector3 &outPos1, Storm::Vector3 &outPos2) const;
		Storm::Vector3 getForceApplied() const;

	public:
		void execute();

	private:
		unsigned int _constraintId;

		physx::PxRigidActor*const _actor1;
		physx::PxRigidActor*const _actor2;

		const Storm::Vector3 _actor1LinkHookOffset;
		const Storm::Vector3 _actor2LinkHookOffset;
		float _hardDistSquared;

		bool _actor1Static;
		bool _actor2Static;
	};
}
