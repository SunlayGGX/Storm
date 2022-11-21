#include "SmokeShader.h"
#include "Camera.h"

#include "IConfigManager.h"
#include "SingletonHolder.h"

#include "SceneSimulationConfig.h"

#include "ResourceMapperGuard.h"
#include "MemoryHelper.h"


namespace
{
	struct ConstantBuffer
	{
		DirectX::XMMATRIX _viewMatrix;
		DirectX::XMMATRIX _projectionMatrix;

		DirectX::XMVECTOR _color;

		float _dimension;
		float _nearPlaneDist;

		const DirectX::XMFLOAT2 _padding;
	};

	static const std::string k_smokeShaderFilePath = "Shaders/SmokeDraw.hlsl";
	static constexpr std::string_view k_smokeVertexShaderFuncName = "smokeVertexShader";
	static constexpr std::string_view k_particleGeometryShaderFuncName = "smokeGeometryShader";
	static constexpr std::string_view k_smokePixelShaderFuncName = "smokePixelShader";

	enum : unsigned int
	{
		k_smokeVertexDataLayoutDescCount = 1
	};

	inline D3D11_INPUT_ELEMENT_DESC* retrieveSmokeInputLayoutElementDesc()
	{
		static D3D11_INPUT_ELEMENT_DESC smokeVertexDataLayoutDesc[k_smokeVertexDataLayoutDescCount];

		D3D11_INPUT_ELEMENT_DESC &currentVertexDataLayoutDesc = smokeVertexDataLayoutDesc[0];
		currentVertexDataLayoutDesc.SemanticName = "POSITION";
		currentVertexDataLayoutDesc.SemanticIndex = 0;
		currentVertexDataLayoutDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT;
		currentVertexDataLayoutDesc.InputSlot = 0;
		currentVertexDataLayoutDesc.AlignedByteOffset = 0;
		currentVertexDataLayoutDesc.InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
		currentVertexDataLayoutDesc.InstanceDataStepRate = 0;

		return smokeVertexDataLayoutDesc;
	}
}


Storm::SmokeShader::SmokeShader(const ComPtr<ID3D11Device> &device, ComPtr<ID3D11ShaderResourceView> &&perlinNoiseTextureSRV) :
	VPShaderBase{ device, k_smokeShaderFilePath, k_smokeVertexShaderFuncName, k_smokeShaderFilePath, k_particleGeometryShaderFuncName, k_smokeShaderFilePath, k_smokePixelShaderFuncName, retrieveSmokeInputLayoutElementDesc(), k_smokeVertexDataLayoutDescCount },
	_perlinNoiseTextureSRV{ std::move(perlinNoiseTextureSRV) }
{
	if (_perlinNoiseTextureSRV == nullptr)
	{
		Storm::throwException<Storm::Exception>("Perlin noise shader resource view should not be null!");
	}

	Storm::ConstantBufferHolder::initialize<ConstantBuffer>(device);
}

void Storm::SmokeShader::setup(const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera, const DirectX::XMVECTOR &color)
{
	// Setup the device context
	this->setupDeviceContext(deviceContext);

	// Write shaders parameters
	{
		D3D11_MAPPED_SUBRESOURCE particleConstantBufferRessource;
		Storm::ResourceMapperGuard mapGuard{ deviceContext, _constantBuffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, particleConstantBufferRessource };

		ConstantBuffer*const ressourceDataPtr = static_cast<ConstantBuffer*>(particleConstantBufferRessource.pData);

		ressourceDataPtr->_viewMatrix = currentCamera.getTransposedViewMatrix();
		ressourceDataPtr->_projectionMatrix = currentCamera.getTransposedProjectionMatrix();

		ressourceDataPtr->_color = color;

		ressourceDataPtr->_nearPlaneDist = currentCamera.getNearPlane();

		ressourceDataPtr->_dimension = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>().getSceneSimulationConfig()._particleRadius * 2.f;
	}

	ID3D11Buffer*const constantBufferTmp = _constantBuffer.Get();
	
	deviceContext->VSSetConstantBuffers(0, 1, &constantBufferTmp);
	deviceContext->GSSetConstantBuffers(0, 1, &constantBufferTmp);
	deviceContext->PSSetConstantBuffers(0, 1, &constantBufferTmp);

	ID3D11ShaderResourceView*const perlinTextureSRV = _perlinNoiseTextureSRV.Get();
	deviceContext->PSSetShaderResources(0, 1, &perlinTextureSRV);
}
