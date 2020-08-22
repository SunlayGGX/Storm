#include "BlowerShader.h"

#include "MemoryHelper.h"

#include "Camera.h"

namespace
{
	struct ConstantBuffer
	{
		DirectX::XMMATRIX _viewMatrix;
		DirectX::XMMATRIX _projectionMatrix;

		DirectX::XMVECTOR _color;
	};

	static const std::string k_blowerShaderFilePath = "Shaders/BlowerDraw.hlsl";
	static constexpr std::string_view k_blowerVertexShaderFuncName = "blowerVertexShader";
	static constexpr std::string_view k_blowerPixelShaderFuncName = "blowerPixelShader";

	enum : unsigned int
	{
		k_blowerVertexDataLayoutDescCount = 1
	};

	inline D3D11_INPUT_ELEMENT_DESC* retrieveBlowerInputLayoutElementDesc()
	{
		static D3D11_INPUT_ELEMENT_DESC bowerVertexDataLayoutDesc[k_blowerVertexDataLayoutDescCount];

		D3D11_INPUT_ELEMENT_DESC &currentVertexDataLayoutDesc = bowerVertexDataLayoutDesc[0];
		currentVertexDataLayoutDesc.SemanticName = "POSITION";
		currentVertexDataLayoutDesc.SemanticIndex = 0;
		currentVertexDataLayoutDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
		currentVertexDataLayoutDesc.InputSlot = 0;
		currentVertexDataLayoutDesc.AlignedByteOffset = 0;
		currentVertexDataLayoutDesc.InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
		currentVertexDataLayoutDesc.InstanceDataStepRate = 0;

		return bowerVertexDataLayoutDesc;
	}
}

Storm::BlowerShader::BlowerShader(const ComPtr<ID3D11Device> &device, const uint32_t indexCount) :
	Storm::VPShaderBase{ device, k_blowerShaderFilePath, k_blowerVertexShaderFuncName, k_blowerShaderFilePath, k_blowerPixelShaderFuncName, retrieveBlowerInputLayoutElementDesc(), k_blowerVertexDataLayoutDescCount }
{
	Storm::ConstantBufferHolder::initialize<ConstantBuffer>(device);
}

void Storm::BlowerShader::setup(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera, const DirectX::XMVECTOR &color)
{
	this->setupDeviceContext(deviceContext);

	// Write shaders parameters
	D3D11_MAPPED_SUBRESOURCE blowerConstantBufferRessource;
	Storm::throwIfFailed(deviceContext->Map(_constantBuffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &blowerConstantBufferRessource));

	ConstantBuffer*const ressourceDataPtr = static_cast<ConstantBuffer*>(blowerConstantBufferRessource.pData);

	ressourceDataPtr->_viewMatrix = currentCamera.getTransposedViewMatrix();
	ressourceDataPtr->_projectionMatrix = currentCamera.getTransposedProjectionMatrix();
	ressourceDataPtr->_color = color;

	deviceContext->Unmap(_constantBuffer.Get(), 0);

	ID3D11Buffer*const constantBufferTmp = _constantBuffer.Get();
	deviceContext->VSSetConstantBuffers(0, 1, &constantBufferTmp);
	deviceContext->PSSetConstantBuffers(0, 1, &constantBufferTmp);
}
