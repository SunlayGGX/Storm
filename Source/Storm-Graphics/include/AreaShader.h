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
		AreaShader(const ComPtr<ID3D11Device> &device, const std::span<const std::string_view> macros);

	public:
		void setup(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera, const DirectX::XMVECTOR &color);

	public:
		void updateWorldMat(const DirectX::XMVECTOR &position, const Storm::Quaternion &rotation, const Storm::Vector3 &scale);

	private:
		DirectX::XMMATRIX _worldMat;
	};
}
