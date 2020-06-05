#pragma once


namespace Storm
{
	struct RigidBodySceneData;
	class IRigidBody;

	class PhysicsRigidBody
	{
	public:
		PhysicsRigidBody(const Storm::RigidBodySceneData &rbSceneData, const std::vector<Storm::Vector3> &vertices);

	public:
		void setRbParent(const std::shared_ptr<Storm::IRigidBody> &boundRbParent);
		std::shared_ptr<Storm::IRigidBody> getRbParent();
		const std::shared_ptr<Storm::IRigidBody>& getRbParent() const;

	private:
		std::shared_ptr<Storm::IRigidBody> _boundParentRb;
	};
}
