#pragma once


namespace Storm
{
	class ConstantBufferHolder
	{
	protected:
		ConstantBufferHolder() = default;
		virtual ~ConstantBufferHolder() = default;

	protected:
		template<class ConstantBufferType>
		void initialize(const ComPtr<ID3D11Device> &device)
		{
			D3D11_BUFFER_DESC constantBufferDesc;
			Storm::ZeroMemories(constantBufferDesc);

			constantBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
			constantBufferDesc.ByteWidth = sizeof(ConstantBuffer);
			constantBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
			constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;

			Storm::throwIfFailed(device->CreateBuffer(&constantBufferDesc, nullptr, &_constantBuffer));
		}

	protected:
		ComPtr<ID3D11Buffer> _constantBuffer;
	};
}
