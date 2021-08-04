#pragma once

#include "ConstantBufferHolder.h"
#include "VPShader.h"


namespace Storm
{
	class GravityShader :
		public Storm::VPShaderBase,
		private Storm::ConstantBufferHolder
	{
	public:
		GravityShader(const ComPtr<ID3D11Device> &device);

	public:
		void draw(const ComPtr<ID3D11DeviceContext> &deviceContext);

		void setup(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera);
	};
}
