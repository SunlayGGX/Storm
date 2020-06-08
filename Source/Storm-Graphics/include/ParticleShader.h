#pragma once

#include "VPShader.h"
#include "ConstantBufferHolder.h"


namespace Storm
{
	// This class would be interfaced with ParticleDraw.hlsl
	class ParticleShader :
		public Storm::VPShaderBase,
		private Storm::ConstantBufferHolder
	{
	public:
		ParticleShader(const ComPtr<ID3D11Device> &device);

	public:
		void setup(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera);
	};
}
