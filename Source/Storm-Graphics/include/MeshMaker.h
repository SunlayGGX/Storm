#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	class CubeMeshMaker : private Storm::NonInstanciable
	{
	public:
		static void generate(const Storm::Vector3 &pos, const Storm::Vector3 &scale, std::vector<Storm::Vector3> &outVertexes, std::vector<uint32_t> &outIndexes);
	};

	class SphereMeshMaker : private Storm::NonInstanciable
	{
	public:
		static void generate(const Storm::Vector3 &pos, const Storm::Vector3 &scale, std::vector<Storm::Vector3> &outVertexes, std::vector<uint32_t> &outIndexes);
	};
}
