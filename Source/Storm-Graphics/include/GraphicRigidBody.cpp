#include "GraphicRigidBody.h"


Storm::GraphicRigidBody::GraphicRigidBody(const std::vector<Storm::Vector3> &vertices, const std::vector<Storm::Vector3> &normals)
{
	for (const auto &vertice : vertices)
	{
		_vertices.emplace_back(vertice.x(), vertice.y(), vertice.z());
	}

	for (const auto &normal : normals)
	{
		_normals.emplace_back(normal.x(), normal.y(), normal.z());
	}
}

void Storm::GraphicRigidBody::setRbParent(const std::shared_ptr<Storm::IRigidBody> &boundRbParent)
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

std::shared_ptr<Storm::IRigidBody> Storm::GraphicRigidBody::getRbParent()
{
	return _boundParentRb;
}

const std::shared_ptr<Storm::IRigidBody>& Storm::GraphicRigidBody::getRbParent() const
{
	return _boundParentRb;
}

