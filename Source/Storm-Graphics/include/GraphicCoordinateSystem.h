#pragma once

#include "IRenderedElement.h"


namespace Storm
{
	class CoordinateSystemShader;
	class Camera;
	class GraphicCoordinateSystem : public Storm::IRenderedElement
	{
	public:
		GraphicCoordinateSystem(const ComPtr<ID3D11Device> &device);

	public:
		void render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera) final override;

	private:
		void setup(const ComPtr<ID3D11DeviceContext> &deviceContext);

	public:
		void show(bool shouldShow);
		void switchShow();

	private:
		ComPtr<ID3D11Buffer> _vertexBuffer;
		ComPtr<ID3D11Buffer> _indexBuffer;

		bool _shouldShow;

		std::unique_ptr<Storm::CoordinateSystemShader> _coordinateSysShader;
	};
}
