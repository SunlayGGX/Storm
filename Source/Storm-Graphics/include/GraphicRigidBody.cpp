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
