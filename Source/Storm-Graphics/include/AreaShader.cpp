#include "AreaShader.h"

#include "Camera.h"

#include "MemoryHelper.h"
#include "ResourceMapperGuard.h"

namespace
{
	struct ConstantBuffer
	{
		DirectX::XMMATRIX _viewMatrix;
		DirectX::XMMATRIX _projectionMatrix;

		DirectX::XMVECTOR _color;
	};

	static const std::string k_areaShaderFilePath = "Shaders/AreaDraw.hlsl";
	static constexpr std::string_view k_areaVertexShaderFuncName = "areaVertexShader";
	static constexpr std::string_view k_areaPixelShaderFuncName = "areaPixelShader";

	enum : unsigned int
	{
		k_areaVertexDataLayoutDescCount = 1
	};

	inline D3D11_INPUT_ELEMENT_DESC* retrieveBlowerInputLayoutElementDesc()
	{
		static D3D11_INPUT_ELEMENT_DESC bowerVertexDataLayoutDesc[k_areaVertexDataLayoutDescCount];

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

Storm::AreaShader::AreaShader(const ComPtr<ID3D11Device> &device, const uint32_t indexCount) :
	Storm::VPShaderBase{ device, k_areaShaderFilePath, k_areaVertexShaderFuncName, k_areaShaderFilePath, k_areaPixelShaderFuncName, retrieveBlowerInputLayoutElementDesc(), k_areaVertexDataLayoutDescCount }
{
	Storm::ConstantBufferHolder::initialize<ConstantBuffer>(device);
}

void Storm::AreaShader::setup(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera, const DirectX::XMVECTOR &color)
{
	this->setupDeviceContext(deviceContext);

	// Write shaders parameters
	{
		D3D11_MAPPED_SUBRESOURCE areaConstantBufferRessource;
		Storm::ResourceMapperGuard mapGuard{ deviceContext, _constantBuffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, areaConstantBufferRessource };

		ConstantBuffer*const ressourceDataPtr = static_cast<ConstantBuffer*>(areaConstantBufferRessource.pData);

		ressourceDataPtr->_viewMatrix = currentCamera.getTransposedViewMatrix();
		ressourceDataPtr->_projectionMatrix = currentCamera.getTransposedProjectionMatrix();
		ressourceDataPtr->_color = color;
	}

	ID3D11Buffer*const constantBufferTmp = _constantBuffer.Get();
	deviceContext->VSSetConstantBuffers(0, 1, &constantBufferTmp);
	deviceContext->PSSetConstantBuffers(0, 1, &constantBufferTmp);
}
