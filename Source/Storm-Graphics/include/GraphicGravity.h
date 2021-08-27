#pragma once

#include "IRenderedElement.h"

namespace Storm
{
	class GravityShader;
	class Camera;
	class GraphicGravity : public Storm::IRenderedElement
	{
	public:
		GraphicGravity(const ComPtr<ID3D11Device> &device, const ComPtr<ID2D1RenderTarget> &hudTarget);

	public:
		void render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera) final override;
		void postRenderUI(const ComPtr<ID2D1RenderTarget> &hudTarget, IDWriteTextFormat*const textFormat, const float viewportWidth, const float viewportHeight) final override;

	private:
		void setup(const ComPtr<ID3D11DeviceContext> &deviceContext);

	public:
		void switchShow();
	
	private:
		bool _visible;
		
		ComPtr<ID3D11Buffer> _vertexBuffer;
		ComPtr<ID3D11Buffer> _indexBuffer;

		ComPtr<ID2D1SolidColorBrush> _direct2DTextSolidBrush;

		std::unique_ptr<Storm::GravityShader> _gravityShader;
	};
}
