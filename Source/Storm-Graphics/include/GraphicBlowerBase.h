#pragma once

#include "IRenderedElement.h"


namespace Storm
{
	enum class BlowerType;
	struct BlowerData;
	class BlowerShader;

	class GraphicBlowerBase : public Storm::IRenderedElement
	{
	protected:
		GraphicBlowerBase(const Storm::BlowerData &blowerData);

	public:
		virtual ~GraphicBlowerBase();

	protected:
		void instantiateShader(const ComPtr<ID3D11Device> &device, const std::vector<Storm::Vector3> &vertexes, const std::vector<uint32_t> &indexes);

	public:
		void render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera) final override;

	private:
		void setupBlower(const ComPtr<ID3D11DeviceContext> &deviceContext);

	public:
		std::size_t getId() const;
		virtual Storm::BlowerType getType() const = 0;

	protected:
		std::size_t _id;

		uint32_t _indexCount;

		ComPtr<ID3D11Buffer> _vertexBuffer;
		ComPtr<ID3D11Buffer> _indexBuffer;

		DirectX::XMMATRIX _blowerWorldMatrix;

		std::unique_ptr<Storm::BlowerShader> _blowerShader;
	};
}
