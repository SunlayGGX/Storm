#include "GridShader.h"

#include "Camera.h"

#include "MemoryHelper.h"


namespace
{
	struct ConstantBuffer
	{
		DirectX::XMMATRIX _viewMatrix;
		DirectX::XMMATRIX _projectionMatrix;

		DirectX::XMVECTOR _gridColor;
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
		currentVertexDataLayoutDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
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
	Storm::ConstantBufferHolder::initialize<ConstantBuffer>(device);
}

void Storm::GridShader::draw(const ComPtr<ID3D11DeviceContext> &deviceContext)
{
	Storm::VPShaderBase::draw(_gridIndexCount, deviceContext);
}

void Storm::GridShader::setup(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera)
{
	// Setup the device context
	this->setupDeviceContext(deviceContext);

	// Write shaders parameters
	D3D11_MAPPED_SUBRESOURCE gridConstantBufferRessource;
	Storm::throwIfFailed(deviceContext->Map(_constantBuffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &gridConstantBufferRessource));

	ConstantBuffer*const ressourceDataPtr = static_cast<ConstantBuffer*>(gridConstantBufferRessource.pData);

	ressourceDataPtr->_viewMatrix = currentCamera.getTransposedViewMatrix();
	ressourceDataPtr->_projectionMatrix = currentCamera.getTransposedProjectionMatrix();

	ressourceDataPtr->_gridColor = DirectX::XMVECTOR{ 0.7f, 0.7f, 0.7f, 1.f };

	deviceContext->Unmap(_constantBuffer.Get(), 0);

	ID3D11Buffer*const constantBufferTmp = _constantBuffer.Get();
	deviceContext->VSSetConstantBuffers(0, 1, &constantBufferTmp);
	deviceContext->PSSetConstantBuffers(0, 1, &constantBufferTmp);
}

