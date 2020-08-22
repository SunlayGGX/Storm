#pragma once

#include "GraphicBlowerBase.h"


namespace Storm
{
	enum class BlowerType;

	template<Storm::BlowerType type, class BlowerMeshMaker>
	class GraphicBlower :
		public Storm::GraphicBlowerBase
	{
	public:
		GraphicBlower(const ComPtr<ID3D11Device> &device, const Storm::BlowerData &blowerData) :
			Storm::GraphicBlowerBase{ blowerData }
		{
			std::vector<Storm::Vector3> vertexes;
			std::vector<uint32_t> indexes;

			BlowerMeshMaker::generate(blowerData, vertexes, indexes);

			this->instantiateShader(device, vertexes, indexes);
		}

	public:
		constexpr Storm::BlowerType getType() const final override
		{
			return type;
		}
	};
}
