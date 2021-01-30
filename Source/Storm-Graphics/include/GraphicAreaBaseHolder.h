#pragma once

#include "IRenderedElement.h"


namespace Storm
{
	class AreaShader;

	class GraphicAreaBaseHolder
	{
	public:
		virtual ~GraphicAreaBaseHolder();

	protected:
		void initializeShader(const ComPtr<ID3D11Device> &device, const std::vector<Storm::Vector3> &vertexes, const std::vector<unsigned int> &indexes);

	protected:
		void setup(const ComPtr<ID3D11DeviceContext> &deviceContext);

	protected:
		uint32_t _indexCount;

		ComPtr<ID3D11Buffer> _vertexBuffer;
		ComPtr<ID3D11Buffer> _indexBuffer;

		std::unique_ptr<Storm::AreaShader> _areaShader;
	};
}
