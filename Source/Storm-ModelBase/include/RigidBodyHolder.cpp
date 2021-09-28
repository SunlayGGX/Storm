#include "RigidBodyHolder.h"

#include "IRigidBody.h"


void Storm::RigidBodyHolder::setRbParent(const std::shared_ptr<Storm::IRigidBody> &boundRbParent)
{
	if (_boundParentRb == nullptr)
	{
		_boundParentRb = boundRbParent;
	}
	else
	{
		Storm::throwException<Storm::Exception>("You're trying to overwrite an existing link. Once it has been done, you're not allowed to set another parent!");
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

const Storm::Vector3& Storm::RigidBodyHolder::getRbPosition() const
{
	// Do not query the one inside _boundParentRb because it isn't thread safe.
	return _cachedPosition;
}

void Storm::RigidBodyHolder::setRbPosition(const Storm::Vector3 &pos)
{
	_cachedPosition = pos;
}

unsigned int Storm::RigidBodyHolder::getID() const
{
	return _boundParentRb->getRigidBodyID();
}
