#pragma once

#include "VPShader.h"
#include "ConstantBufferHolder.h"


namespace Storm
{
	class BlowerShader :
		public Storm::VPShaderBase,
		private Storm::ConstantBufferHolder
	{
	public:
		BlowerShader(const ComPtr<ID3D11Device> &device, const uint32_t indexCount, const DirectX::XMMATRIX &blowerWorldMatrix);
	};
}
