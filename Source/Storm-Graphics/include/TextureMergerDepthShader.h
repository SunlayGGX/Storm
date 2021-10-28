#pragma once

#include "VPShader.h"
#include "ConstantBufferHolder.h"


namespace Storm
{
	class TextureMergerDepthShader :
		public Storm::VPShaderBase,
		private Storm::ConstantBufferHolder
	{
	public:
		TextureMergerDepthShader(const ComPtr<ID3D11Device> &device);
	};
}
