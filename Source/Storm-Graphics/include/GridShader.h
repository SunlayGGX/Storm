#pragma once

#include "VPShader.h"
#include "ConstantBufferHolder.h"


namespace Storm
{
	// This class would be interfaced with Grid.hlsl
	class GridShader :
		public Storm::VPShaderBase,
		private Storm::ConstantBufferHolder
	{
	public:
		GridShader(const ComPtr<ID3D11Device> &device, unsigned int indexCount);

	public:
		void draw(const ComPtr<ID3D11DeviceContext> &deviceContext);

		void setup(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera);

	private:
		unsigned int _gridIndexCount;
	};
}
