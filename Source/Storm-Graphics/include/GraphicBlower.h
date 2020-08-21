#pragma once


namespace Storm
{
	enum class BlowerType;

	template<Storm::BlowerType type, class MeshMaker>
	class GraphicBlower :
		public Storm::GraphicBlowerBase
	{
	public:
		GraphicBlower(const ComPtr<ID3D11Device> &device, const std::size_t index, const Storm::BlowerData &blowerData) :
			Storm::GraphicBlowerBase{ index, blowerData }
		{

		}
	};
}
