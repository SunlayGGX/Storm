#pragma once


namespace Storm
{
	class IRigidBody;

	class RigidBodyHolder
	{
	public:
		RigidBodyHolder();
		virtual ~RigidBodyHolder() = default;

	public:
		void setRbParent(const std::shared_ptr<Storm::IRigidBody> &boundRbParent);
		std::shared_ptr<Storm::IRigidBody> getRbParent();
		const std::shared_ptr<Storm::IRigidBody>& getRbParent() const;

		const Storm::Vector3& getRbPosition() const;
		void setRbPosition(const Storm::Vector3 &pos);

		unsigned int getID() const noexcept;

		bool positionDirty() const noexcept;

	private:
		std::shared_ptr<Storm::IRigidBody> _boundParentRb;
		Storm::Vector3 _cachedPosition;
		bool _positionChanged;
	};
}
