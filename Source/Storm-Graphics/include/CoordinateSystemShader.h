#pragma once

#include "VPShader.h"
#include "ConstantBufferHolder.h"


namespace Storm
{
	// This class would be interfaced with CoordinateSystem.hlsl
	class CoordinateSystemShader :
		public Storm::VPShaderBase,
		private Storm::ConstantBufferHolder
	{
	public:
		CoordinateSystemShader(const ComPtr<ID3D11Device> &device, unsigned int indexCount);

	public:
		void draw(const ComPtr<ID3D11DeviceContext> &deviceContext);

		void setup(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera);

	private:
		const unsigned int _indexCount;
	};
}
