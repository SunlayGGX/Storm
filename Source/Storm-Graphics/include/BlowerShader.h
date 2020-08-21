#pragma once


namespace Storm
{
	class BlowerShader
	{
	public:
		BlowerShader(const ComPtr<ID3D11Device> &device, const uint32_t indexCount, const DirectX::XMMATRIX &blowerWorldMatrix);
	};
}
