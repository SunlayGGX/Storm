#pragma once

#include "VPShader.h"


namespace Storm
{
	// This class would be interfaced with MeshDraw.hlsl
	class MeshShader : public Storm::VPShaderBase
	{
	public:
		MeshShader(const ComPtr<ID3D11Device> &device);

	public:
		void draw(int indexCount, const ComPtr<ID3D11DeviceContext> &deviceContext);
		void setup(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera, const DirectX::XMMATRIX &transposedTransform);

	private:
		ComPtr<ID3D11Buffer> _constantBuffer;
	};
}
