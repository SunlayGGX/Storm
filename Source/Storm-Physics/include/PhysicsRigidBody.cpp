#include "PhysicsRigidBody.h"

#include "ThrowException.h"



Storm::PhysicsRigidBody::PhysicsRigidBody(const Storm::RigidBodySceneData &rbSceneData, const std::vector<Storm::Vector3> &vertices)
{
	// TODO
}

void Storm::PhysicsRigidBody::setRbParent(const std::shared_ptr<Storm::IRigidBody> &boundRbParent)
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

std::shared_ptr<Storm::IRigidBody> Storm::PhysicsRigidBody::getRbParent()
{
	return _boundParentRb;
}

const std::shared_ptr<Storm::IRigidBody>& Storm::PhysicsRigidBody::getRbParent() const
{
	return _boundParentRb;
}
