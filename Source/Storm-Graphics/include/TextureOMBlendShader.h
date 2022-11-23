#pragma once

#include "VPShader.h"
#include "ConstantBufferHolder.h"


namespace Storm
{
	// This class would be interfaced with SmokeDraw.hlsl
	class TextureOMBlendShader :
		public Storm::VPShaderBase,
		private Storm::ConstantBufferHolder
	{
	public:
		TextureOMBlendShader(const ComPtr<ID3D11Device> &device);

	public:
		void setup(const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera, ID3D11ShaderResourceView* toBlend);
	};
}
