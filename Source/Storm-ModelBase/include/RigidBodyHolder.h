#pragma once


namespace Storm
{
	class IRigidBody;

	class RigidBodyHolder
	{
	public:
		virtual ~RigidBodyHolder() = default;

	public:
		void setRbParent(const std::shared_ptr<Storm::IRigidBody> &boundRbParent);
		std::shared_ptr<Storm::IRigidBody> getRbParent();
		const std::shared_ptr<Storm::IRigidBody>& getRbParent() const;

	private:
		std::shared_ptr<Storm::IRigidBody> _boundParentRb;
	};
}
