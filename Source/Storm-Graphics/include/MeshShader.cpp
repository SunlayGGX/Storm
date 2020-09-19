#include "MeshShader.h"

#include "Camera.h"

#include "MemoryHelper.h"
#include "ResourceMapperGuard.h"

namespace
{
	struct ConstantBuffer
	{
		DirectX::XMMATRIX _worldMatrix;
		DirectX::XMMATRIX _viewMatrix;
		DirectX::XMMATRIX _projectionMatrix;

		DirectX::XMVECTOR _meshColor;
	};

	static const std::string k_meshShaderFilePath = "Shaders/MeshDraw.hlsl";
	static constexpr std::string_view k_meshVertexShaderFuncName = "meshVertexShader";
	static constexpr std::string_view k_meshPixelShaderFuncName = "meshPixelShader";

	enum : unsigned int
	{
		k_meshVertexDataLayoutDescCount = 1
	};

	inline D3D11_INPUT_ELEMENT_DESC* retrieveMeshInputLayoutElementDesc()
	{
		static D3D11_INPUT_ELEMENT_DESC meshVertexDataLayoutDesc[k_meshVertexDataLayoutDescCount];

		D3D11_INPUT_ELEMENT_DESC &currentVertexDataLayoutDesc = meshVertexDataLayoutDesc[0];
		currentVertexDataLayoutDesc.SemanticName = "POSITION";
		currentVertexDataLayoutDesc.SemanticIndex = 0;
		currentVertexDataLayoutDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
		currentVertexDataLayoutDesc.InputSlot = 0;
		currentVertexDataLayoutDesc.AlignedByteOffset = 0;
		currentVertexDataLayoutDesc.InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
		currentVertexDataLayoutDesc.InstanceDataStepRate = 0;

		return meshVertexDataLayoutDesc;
	}
}

Storm::MeshShader::MeshShader(const ComPtr<ID3D11Device> &device) :
	Storm::VPShaderBase{ device, k_meshShaderFilePath, k_meshVertexShaderFuncName, k_meshShaderFilePath, k_meshPixelShaderFuncName, retrieveMeshInputLayoutElementDesc(), k_meshVertexDataLayoutDescCount }
{
	Storm::ConstantBufferHolder::initialize<ConstantBuffer>(device);
}

void Storm::MeshShader::setup(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera, const DirectX::XMMATRIX &transposedTransform)
{
	// Setup the device context
	this->setupDeviceContext(deviceContext);

	// Write shaders parameters
	{
		D3D11_MAPPED_SUBRESOURCE meshConstantBufferRessource;
		Storm::ResourceMapperGuard mapGuard{ deviceContext, _constantBuffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, meshConstantBufferRessource };

		ConstantBuffer*const ressourceDataPtr = static_cast<ConstantBuffer*>(meshConstantBufferRessource.pData);

		ressourceDataPtr->_worldMatrix = transposedTransform;
		ressourceDataPtr->_viewMatrix = currentCamera.getTransposedViewMatrix();
		ressourceDataPtr->_projectionMatrix = currentCamera.getTransposedProjectionMatrix();

		ressourceDataPtr->_meshColor = DirectX::XMVECTOR{ 0.2f, 0.6f, 0.6f, 1.f };
	}

	ID3D11Buffer*const constantBufferTmp = _constantBuffer.Get();
	deviceContext->VSSetConstantBuffers(0, 1, &constantBufferTmp);
	deviceContext->PSSetConstantBuffers(0, 1, &constantBufferTmp);
}

