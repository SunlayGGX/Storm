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

		float _screenSpaceXOffset;
		float _screenSpaceYOffset;

		float _maxAxisLengthScreenUnit;

		float _midThickness;
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

	// Write shaders parameters
	{
		D3D11_MAPPED_SUBRESOURCE coordinateSystemConstantBufferRessource;
		Storm::ResourceMapperGuard mapGuard{ deviceContext, _constantBuffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, coordinateSystemConstantBufferRessource };

		ConstantBuffer*const ressourceDataPtr = static_cast<ConstantBuffer*>(coordinateSystemConstantBufferRessource.pData);

		ressourceDataPtr->_viewMatrix = currentCamera.getTransposedViewMatrix();
		ressourceDataPtr->_projMatrix = currentCamera.getTransposedProjectionMatrix();

		// Since screen space coordinate are between -1,-1 (bottom-left) and 1,1 (top-right).
		// We'll set the axis coordinate system to the bottom right of the screen.
		// Then, the axis will be displayed in a R² domain between { x C [0.75, 0.95], y C [-0.75, -0.95] } 
		ressourceDataPtr->_screenSpaceXOffset = 0.85f;
		ressourceDataPtr->_screenSpaceYOffset = -0.85f;
		ressourceDataPtr->_maxAxisLengthScreenUnit = 0.1f;

		ressourceDataPtr->_midThickness = 0.0075f;
	}

	ID3D11Buffer*const constantBufferTmp = _constantBuffer.Get();
	deviceContext->VSSetConstantBuffers(0, 1, &constantBufferTmp);
	deviceContext->GSSetConstantBuffers(0, 1, &constantBufferTmp);
	//deviceContext->PSSetConstantBuffers(0, 1, &constantBufferTmp);
}
