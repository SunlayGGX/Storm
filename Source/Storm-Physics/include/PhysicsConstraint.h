#pragma once


namespace Storm
{
	struct ConstraintData;
	class CordJoint;
	class UIFieldContainer;

	// Link constraint... Some other constraint type exist but won't I use them (I don't need them, YAGNI), so PhysicsConstraint == Distance joint.
	class PhysicsConstraint
	{
	public:
		PhysicsConstraint(const Storm::ConstraintData &data, physx::PxRigidActor* actor1, physx::PxRigidActor* actor2);
		~PhysicsConstraint();

	public:
		void appendJointPositionToArray(std::vector<Storm::Vector3> &inOutJointPositions);
		std::size_t getID() const noexcept;

	public:
		void setCordDistance(float distance);

	private:
		const std::size_t _id;
		std::unique_ptr<Storm::CordJoint> _cordJointPtr;
		bool _shouldVisualize;
		const float _maxDistance;

		std::wstring _distanceWStr;
		const std::wstring _distanceFieldNameWStr;
		std::unique_ptr<Storm::UIFieldContainer> _fields;
	};
}
