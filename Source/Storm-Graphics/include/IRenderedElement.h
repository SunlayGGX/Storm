#pragma once


namespace Storm
{
	class Camera;

	class IRenderedElement
	{
	public:
		virtual ~IRenderedElement() = default;

	public:
		virtual void render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera) = 0;
		virtual void postRenderUI(const ComPtr<ID2D1RenderTarget> &/*hudTarget*/, IDWriteTextFormat*const /*textFormat*/, const float /*viewportWidth*/, const float /*viewportHeight*/) {};
	};
}
