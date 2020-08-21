#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	class BasicMeshGenerator : private Storm::NonInstanciable
	{
	public:
		static void generateCube(const Storm::Vector3 &position, const Storm::Vector3 &dimension, std::vector<Storm::Vector3> &inOutVertexes, std::vector<uint32_t> &inOutIndexes);
		static void generateSphere(const Storm::Vector3 &position, const float radius, std::vector<Storm::Vector3> &inOutVertexes, std::vector<uint32_t> &inOutIndexes);
	};
}