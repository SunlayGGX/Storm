#pragma once

#include "RigidBodyHolder.h"


namespace Storm
{
	class GraphicRigidBody : public Storm::RigidBodyHolder
	{
	public:
		GraphicRigidBody(const std::vector<Storm::Vector3> &vertices, const std::vector<Storm::Vector3> &normals);

	private:
		std::vector<DirectX::XMFLOAT3> _vertices;
		std::vector<DirectX::XMFLOAT3> _normals;
	};
}
