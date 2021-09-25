#include "GravityShader.h"

#include "Camera.h"

#include "MemoryHelper.h"
#include "ResourceMapperGuard.h"


namespace
{
	struct ConstantBuffer
	{
		DirectX::XMMATRIX _viewProjMatrix;

		float _screenSpaceXOffset;
		float _screenSpaceYOffset;

		float _maxAxisLengthScreenUnit;

		float _midThickness;
	};

	static const std::string k_gravityArrowShaderFilePath = "Shaders/GravityArrow.hlsl";
	static constexpr std::string_view k_gravityArrowVertexShaderFuncName = "gravityArrowVertexShader";
	static constexpr std::string_view k_gravityArrowGeometryShaderFuncName = "gravityArrowGeometryShader";
	static constexpr std::string_view k_gravityArrowPixelShaderFuncName = "gravityArrowPixelShader";

	enum : unsigned int
	{
		k_gravityVertexDataLayoutDescCount = 2
	};

	inline D3D11_INPUT_ELEMENT_DESC* retrieveGravityArrowInputLayoutElementDesc()
	{
		static D3D11_INPUT_ELEMENT_DESC gravityVertexDataLayoutDesc[k_gravityVertexDataLayoutDescCount];

		{
			D3D11_INPUT_ELEMENT_DESC &currentVertexDataLayoutDesc = gravityVertexDataLayoutDesc[0];
			currentVertexDataLayoutDesc.SemanticName = "POSITION";
			currentVertexDataLayoutDesc.SemanticIndex = 0;
			currentVertexDataLayoutDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
			currentVertexDataLayoutDesc.InputSlot = 0;
			currentVertexDataLayoutDesc.AlignedByteOffset = 0;
			currentVertexDataLayoutDesc.InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
			currentVertexDataLayoutDesc.InstanceDataStepRate = 0;
		}

		{
			D3D11_INPUT_ELEMENT_DESC &currentVertexDataLayoutDesc = gravityVertexDataLayoutDesc[1];
			currentVertexDataLayoutDesc.SemanticName = "COLOR";
			currentVertexDataLayoutDesc.SemanticIndex = 0;
			currentVertexDataLayoutDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
			currentVertexDataLayoutDesc.InputSlot = 0;
			currentVertexDataLayoutDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			currentVertexDataLayoutDesc.InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
			currentVertexDataLayoutDesc.InstanceDataStepRate = 0;
		}

		return gravityVertexDataLayoutDesc;
	}
}


Storm::GravityShader::GravityShader(const ComPtr<ID3D11Device> &device) :
	Storm::VPShaderBase{ device, k_gravityArrowShaderFilePath, k_gravityArrowVertexShaderFuncName, k_gravityArrowShaderFilePath, k_gravityArrowGeometryShaderFuncName, k_gravityArrowShaderFilePath, k_gravityArrowPixelShaderFuncName, retrieveGravityArrowInputLayoutElementDesc(), k_gravityVertexDataLayoutDescCount }
{
	Storm::ConstantBufferHolder::initialize<ConstantBuffer>(device);
}

void Storm::GravityShader::draw(const ComPtr<ID3D11DeviceContext>& deviceContext)
{
	Storm::VPShaderBase::draw(1, deviceContext);
}

void Storm::GravityShader::setup(const ComPtr<ID3D11Device>& device, const ComPtr<ID3D11DeviceContext>& deviceContext, const Storm::Camera& currentCamera)
{
	// Setup the device context
	this->setupDeviceContext(deviceContext);

	// Write shaders parameters
	{
		D3D11_MAPPED_SUBRESOURCE coordinateSystemConstantBufferRessource;
		Storm::ResourceMapperGuard mapGuard{ deviceContext, _constantBuffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, coordinateSystemConstantBufferRessource };

		ConstantBuffer*const ressourceDataPtr = static_cast<ConstantBuffer*>(coordinateSystemConstantBufferRessource.pData);

		ressourceDataPtr->_viewProjMatrix = currentCamera.getTransposedViewProjMatrix();

		// Since screen space coordinate are between -1,-1 (bottom-left) and 1,1 (top-right).
		// We'll set the axis coordinate system to the bottom right of the screen.
		// Then, the axis will be displayed in a R² domain between { x C [-0.75, -0.95], y C [0.75, 0.95] }
		const std::pair<float, float> drawLocation{ this->getDrawLocation() };
		ressourceDataPtr->_screenSpaceXOffset = drawLocation.first;
		ressourceDataPtr->_screenSpaceYOffset = drawLocation.second;
		ressourceDataPtr->_maxAxisLengthScreenUnit = this->getAxisLengthUnit();

		ressourceDataPtr->_midThickness = 0.0075f;
	}

	ID3D11Buffer*const constantBufferTmp = _constantBuffer.Get();
	deviceContext->VSSetConstantBuffers(0, 1, &constantBufferTmp);
	deviceContext->GSSetConstantBuffers(0, 1, &constantBufferTmp);
}

std::pair<float, float> Storm::GravityShader::getDrawLocation() const
{
	return { -0.85f, 0.85f };
}

float Storm::GravityShader::getAxisLengthUnit() const
{
	return 0.1f;
}
