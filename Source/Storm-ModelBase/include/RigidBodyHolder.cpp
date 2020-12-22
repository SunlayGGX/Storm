#include "RigidBodyHolder.h"


void Storm::RigidBodyHolder::setRbParent(const std::shared_ptr<Storm::IRigidBody> &boundRbParent)
{
	if (_boundParentRb == nullptr)
	{
		_boundParentRb = boundRbParent;
	}
	else
	{
		Storm::throwException<std::exception>("You're trying to overwrite an existing link. Once it has been done, you're not allowed to set another parent!");
	}
}

std::shared_ptr<Storm::IRigidBody> Storm::RigidBodyHolder::getRbParent()
{
	return _boundParentRb;
}

const std::shared_ptr<Storm::IRigidBody>& Storm::RigidBodyHolder::getRbParent() const
{
	return _boundParentRb;
}
