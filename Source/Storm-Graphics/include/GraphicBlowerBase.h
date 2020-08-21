#pragma once

#include "IRenderedElement.h"


namespace Storm
{
	struct BlowerData;
	class BlowerShader;

	class GraphicBlowerBase : public Storm::IRenderedElement
	{
	protected:
		GraphicBlowerBase(const std::size_t index, const Storm::BlowerData &blowerData);
		virtual ~GraphicBlowerBase();

	protected:
		void instantiateShader(const ComPtr<ID3D11Device> &device, const std::vector<Storm::Vector3> &vertexes, const std::vector<uint32_t> &indexes);

	public:
		std::size_t getIndex() const;

	protected:
		std::size_t _index;

		ComPtr<ID3D11Buffer> _vertexBuffer;
		ComPtr<ID3D11Buffer> _indexBuffer;

		std::unique_ptr<Storm::BlowerShader> _blowerShader;
	};
}
