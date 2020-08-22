#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	struct BlowerData;

	class BlowerCubeMeshMaker : private Storm::NonInstanciable
	{
	public:
		static void generate(const Storm::BlowerData &blowerData, std::vector<Storm::Vector3> &outVertexes, std::vector<uint32_t> &outIndexes);
	};

	class BlowerSphereMeshMaker : private Storm::NonInstanciable
	{
	public:
		static void generate(const Storm::BlowerData &blowerData, std::vector<Storm::Vector3> &outVertexes, std::vector<uint32_t> &outIndexes);
	};
}
