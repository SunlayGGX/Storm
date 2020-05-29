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
	};
}
