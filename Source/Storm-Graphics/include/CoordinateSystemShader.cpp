#include "CoordinateSystemShader.h"

#include "MemoryHelper.h"
#include "ResourceMapperGuard.h"
#include "IConfigManager.h"
#include "SingletonHolder.h"
#include "SceneGraphicConfig.h"
#include "Camera.h"


namespace
{
	struct ConstantBuffer
	{
		DirectX::XMMATRIX _viewMatrix;
		DirectX::XMMATRIX _projMatrix;

		DirectX::XMFLOAT3 _originPosition;

		float _midThickness;
		float _nearPlanePos;
	};

	static const std::string k_coordinateSysShaderFilePath = "Shaders/CoordinateSystem.hlsl";
	static constexpr std::string_view k_coordinateSysVertexShaderFuncName = "coordinateSystemVertexShader";
	static constexpr std::string_view k_coordinateSysGeometryShaderFuncName = "coordinateSystemGeometryShader";
	static constexpr std::string_view k_coordinateSysPixelShaderFuncName = "coordinateSystemPixelShader";

	enum : unsigned int
	{
		k_coordinateSysVertexDataLayoutDescCount = 2
	};

	inline D3D11_INPUT_ELEMENT_DESC* retrieveCoordinateSystemInputLayoutElementDesc()
	{
		static D3D11_INPUT_ELEMENT_DESC coordinateSysVertexDataLayoutDesc[k_coordinateSysVertexDataLayoutDescCount];

		{
			D3D11_INPUT_ELEMENT_DESC &currentVertexDataLayoutDesc = coordinateSysVertexDataLayoutDesc[0];
			currentVertexDataLayoutDesc.SemanticName = "POSITION";
			currentVertexDataLayoutDesc.SemanticIndex = 0;
			currentVertexDataLayoutDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
			currentVertexDataLayoutDesc.InputSlot = 0;
			currentVertexDataLayoutDesc.AlignedByteOffset = 0;
			currentVertexDataLayoutDesc.InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
			currentVertexDataLayoutDesc.InstanceDataStepRate = 0;
		}

		{
			D3D11_INPUT_ELEMENT_DESC &currentVertexDataLayoutDesc = coordinateSysVertexDataLayoutDesc[1];
			currentVertexDataLayoutDesc.SemanticName = "COLOR";
			currentVertexDataLayoutDesc.SemanticIndex = 0;
			currentVertexDataLayoutDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
			currentVertexDataLayoutDesc.InputSlot = 0;
			currentVertexDataLayoutDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			currentVertexDataLayoutDesc.InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
			currentVertexDataLayoutDesc.InstanceDataStepRate = 0;
		}

		return coordinateSysVertexDataLayoutDesc;
	}
}


Storm::CoordinateSystemShader::CoordinateSystemShader(const ComPtr<ID3D11Device> &device, unsigned int indexCount) :
	Storm::VPShaderBase{ device, k_coordinateSysShaderFilePath, k_coordinateSysVertexShaderFuncName, k_coordinateSysShaderFilePath, k_coordinateSysGeometryShaderFuncName, k_coordinateSysShaderFilePath, k_coordinateSysPixelShaderFuncName, retrieveCoordinateSystemInputLayoutElementDesc(), k_coordinateSysVertexDataLayoutDescCount },
	_indexCount{ indexCount }
{
	Storm::ConstantBufferHolder::initialize<ConstantBuffer>(device);
}

void Storm::CoordinateSystemShader::draw(const ComPtr<ID3D11DeviceContext> &deviceContext)
{
	Storm::VPShaderBase::draw(_indexCount, deviceContext);
}

void Storm::CoordinateSystemShader::setup(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera)
{
	// Setup the device context
	this->setupDeviceContext(deviceContext);

	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const Storm::SceneGraphicConfig &graphicConfig = configMgr.getSceneGraphicConfig();

	// Write shaders parameters
	{
		D3D11_MAPPED_SUBRESOURCE particleForcesConstantBufferRessource;
		Storm::ResourceMapperGuard mapGuard{ deviceContext, _constantBuffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, particleForcesConstantBufferRessource };

		ConstantBuffer*const ressourceDataPtr = static_cast<ConstantBuffer*>(particleForcesConstantBufferRessource.pData);

		ressourceDataPtr->_viewMatrix = currentCamera.getTransposedViewMatrix();
		ressourceDataPtr->_projMatrix = currentCamera.getTransposedProjectionMatrix();
		ressourceDataPtr->_originPosition = DirectX::XMFLOAT3{ 0.f, 0.f, 0.f };
		ressourceDataPtr->_midThickness = 0.1f;
		ressourceDataPtr->_nearPlanePos = currentCamera.getNearPlane();
	}

	ID3D11Buffer*const constantBufferTmp = _constantBuffer.Get();
	deviceContext->VSSetConstantBuffers(0, 1, &constantBufferTmp);
	deviceContext->GSSetConstantBuffers(0, 1, &constantBufferTmp);
	//deviceContext->PSSetConstantBuffers(0, 1, &constantBufferTmp);
}
