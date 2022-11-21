#pragma once

#include "VPShader.h"
#include "ConstantBufferHolder.h"


namespace Storm
{
	// This class would be interfaced with SmokeDraw.hlsl
	class SmokeShader :
		public Storm::VPShaderBase,
		private Storm::ConstantBufferHolder
	{
	public:
		SmokeShader(const ComPtr<ID3D11Device> &device, ComPtr<ID3D11ShaderResourceView> &&perlinNoiseTextureSRV);

	public:
		void setup(const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera);

	private:
		ComPtr<ID3D11ShaderResourceView> _perlinNoiseTextureSRV;
	};
}
