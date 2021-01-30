#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	class BasicMeshGenerator : private Storm::NonInstanciable
	{
	public:
		// Smoothed because normals are averages. This does not produce "hard edge" (a different normal per face)... Said otherwise, the 3 vertexes on a same triangle won't have the same normal. The advantage is the smoothed effect and we can reduce the size of the mesh. 
		static void generateSmoothedCube(const Storm::Vector3 &position, const Storm::Vector3 &dimension, std::vector<Storm::Vector3> &inOutVertexes, std::vector<uint32_t> &inOutIndexes, std::vector<Storm::Vector3>*const inOutNormals = nullptr);
		static void generateSphere(const Storm::Vector3 &position, const float radius, std::vector<Storm::Vector3> &inOutVertexes, std::vector<uint32_t> &inOutIndexes, std::vector<Storm::Vector3>*const inOutNormals = nullptr);
		static void generateCone(const Storm::Vector3 &position, const float upRadius, const float downRadius, const float height, std::vector<Storm::Vector3> &inOutVertexes, std::vector<uint32_t> &inOutIndexes, std::vector<Storm::Vector3>*const inOutNormals = nullptr);
		static void generateCylinder(const Storm::Vector3 &position, const float radius, const float height, std::vector<Storm::Vector3> &inOutVertexes, std::vector<uint32_t> &inOutIndexes, std::vector<Storm::Vector3>*const inOutNormals = nullptr);
	};
}
