#pragma once

#include "VPShader.h"
#include "ConstantBufferHolder.h"


namespace Storm
{
	class ConstraintShader :
		public Storm::VPShaderBase,
		private Storm::ConstantBufferHolder
	{
	public:
		ConstraintShader(const ComPtr<ID3D11Device> &device);

	public:
		void setup(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera);
	};
}
