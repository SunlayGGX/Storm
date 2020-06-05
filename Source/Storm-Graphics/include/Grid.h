#pragma once

#include "IRenderedElement.h"


namespace Storm
{
	class GridShader;
	class Camera;
	class Grid : public Storm::IRenderedElement
	{
	public:
		// We will draw a grid from 0,0,0 to maxPt... The diagonal point is retrieve by mirroring on y plane (point symmetry in plane y) the maxPt from the origin 0,0,0
		// It means that if maxPt is equal to 5,1,6, then we will create a grid from -5,1,-6 to 5,1,6. Of course, the line would be spaced of 1...
		Grid(const ComPtr<ID3D11Device> &device, Storm::Vector3 maxPt);

	public:
		void render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera) final override;

	private:
		void setupGrid(const ComPtr<ID3D11DeviceContext> &deviceContext);

	private:
		ComPtr<ID3D11Buffer> _vertexBuffer;
		ComPtr<ID3D11Buffer> _indexBuffer;

		std::unique_ptr<Storm::GridShader> _gridShader;
	};
}
