#pragma once

#include "VPShader.h"
#include "ConstantBufferHolder.h"


namespace Storm
{
	class AreaShader :
		public Storm::VPShaderBase,
		private Storm::ConstantBufferHolder
	{
	public:
		AreaShader(const ComPtr<ID3D11Device> &device, const uint32_t indexCount);

	public:
		void setup(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera, const DirectX::XMVECTOR &color);
	};
}
