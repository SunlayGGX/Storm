#include "GridShader.h"

#include "Camera.h"

#include "MemoryHelper.h"


namespace
{
	struct ConstantBuffer
	{
		DirectX::XMMATRIX _worldMatrix;
		DirectX::XMMATRIX _viewMatrix;
		DirectX::XMMATRIX _projectionMatrix;

		DirectX::XMVECTOR _gridColor;

		DirectX::XMVECTOR _padding1;
		DirectX::XMVECTOR _padding2;
		DirectX::XMVECTOR _padding3;
	};

	static const std::string k_gridShaderFilePath = "Shaders/Grid.hlsl";
	static constexpr std::string_view k_gridVertexShaderFuncName = "gridVertexShader";
	static constexpr std::string_view k_gridPixelShaderFuncName = "gridPixelShader";

	enum : unsigned int
	{
		k_gridVertexDataLayoutDescCount = 1
	};

	inline D3D11_INPUT_ELEMENT_DESC* retrieveGridInputLayoutElementDesc()
	{
		static D3D11_INPUT_ELEMENT_DESC gridVertexDataLayoutDesc[k_gridVertexDataLayoutDescCount];

		D3D11_INPUT_ELEMENT_DESC &currentVertexDataLayoutDesc = gridVertexDataLayoutDesc[0];
		currentVertexDataLayoutDesc.SemanticName = "POSITION";
		currentVertexDataLayoutDesc.SemanticIndex = 0;
		currentVertexDataLayoutDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT;
		currentVertexDataLayoutDesc.InputSlot = 0;
		currentVertexDataLayoutDesc.AlignedByteOffset = 0;
		currentVertexDataLayoutDesc.InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
		currentVertexDataLayoutDesc.InstanceDataStepRate = 0;

		return gridVertexDataLayoutDesc;
	}
}


Storm::GridShader::GridShader(const ComPtr<ID3D11Device> &device, unsigned int indexCount) :
	Storm::VPShaderBase{ device, k_gridShaderFilePath, k_gridVertexShaderFuncName, k_gridShaderFilePath, k_gridPixelShaderFuncName, retrieveGridInputLayoutElementDesc(), k_gridVertexDataLayoutDescCount },
	_gridIndexCount{ indexCount }
{
	D3D11_BUFFER_DESC constantBufferDesc;
	Storm::ZeroMemories(constantBufferDesc);

	constantBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
	constantBufferDesc.ByteWidth = sizeof(ConstantBuffer);
	constantBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;

	Storm::throwIfFailed(device->CreateBuffer(&constantBufferDesc, nullptr, &_constantBuffer));
}

void Storm::GridShader::render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera)
{
	// Write shaders parameters

	D3D11_MAPPED_SUBRESOURCE gridConstantBufferRessource;
	Storm::throwIfFailed(deviceContext->Map(_constantBuffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &gridConstantBufferRessource));

	ConstantBuffer*const ressourceDataPtr = static_cast<ConstantBuffer*>(gridConstantBufferRessource.pData);
	
	ressourceDataPtr->_worldMatrix = currentCamera.getTransposedWorldMatrix();
	ressourceDataPtr->_viewMatrix = currentCamera.getTransposedViewMatrix();
	ressourceDataPtr->_projectionMatrix = currentCamera.getTransposedProjectionMatrix();

	deviceContext->Unmap(_constantBuffer.Get(), 0);

	ID3D11Buffer*const constantBufferTmp = _constantBuffer.Get();
	deviceContext->VSSetConstantBuffers(0, 1, &constantBufferTmp);

	// Setup the device context
	this->setupDeviceContext(deviceContext);

	deviceContext->DrawIndexed(_gridIndexCount, 0, 0);
}

