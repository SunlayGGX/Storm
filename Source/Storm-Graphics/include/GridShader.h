#pragma once

#include "VPShader.h"


namespace Storm
{
	// This class would be interfaced with Grid.hlsl
	class GridShader : public Storm::VPShaderBase
	{
	public:
		GridShader(const ComPtr<ID3D11Device> &device, unsigned int indexCount);

	public:
		void render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera) final override;

	private:
		ComPtr<ID3D11Buffer> _constantBuffer;
		unsigned int _gridIndexCount;
	};
}
