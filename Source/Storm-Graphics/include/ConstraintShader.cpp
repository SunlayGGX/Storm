#include "ConstraintShader.h"

#include "Camera.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "SceneGraphicConfig.h"

#include "MemoryHelper.h"
#include "ResourceMapperGuard.h"


namespace
{
	struct ConstantBuffer
	{
		DirectX::XMMATRIX _viewMatrix;
		DirectX::XMMATRIX _projMatrix;

		DirectX::XMVECTOR _color;
		float _midThickness;
	};

	static const std::string k_constraintShaderFilePath = "Shaders/ConstraintsDraw.hlsl";
	static constexpr std::string_view k_constraintVertexShaderFuncName = "constraintVertexShader";
	static constexpr std::string_view k_constraintGeometryShaderFuncName = "constraintGeometryShader";
	static constexpr std::string_view k_constraintPixelShaderFuncName = "constraintPixelShader";

	enum : unsigned int
	{
		k_constraintVertexDataLayoutDescCount = 1
	};

	inline D3D11_INPUT_ELEMENT_DESC* retrieveConstraintInputLayoutElementDesc()
	{
		static D3D11_INPUT_ELEMENT_DESC constraintVertexDataLayoutDesc[k_constraintVertexDataLayoutDescCount];

		{
			D3D11_INPUT_ELEMENT_DESC &currentVertexDataLayoutDesc = constraintVertexDataLayoutDesc[0];
			currentVertexDataLayoutDesc.SemanticName = "POSITION";
			currentVertexDataLayoutDesc.SemanticIndex = 0;
			currentVertexDataLayoutDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT;
			currentVertexDataLayoutDesc.InputSlot = 0;
			currentVertexDataLayoutDesc.AlignedByteOffset = 0;
			currentVertexDataLayoutDesc.InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
			currentVertexDataLayoutDesc.InstanceDataStepRate = 0;
		}

		return constraintVertexDataLayoutDesc;
	}
}


Storm::ConstraintShader::ConstraintShader(const ComPtr<ID3D11Device> &device) :
	Storm::VPShaderBase{ device, k_constraintShaderFilePath, k_constraintVertexShaderFuncName, k_constraintShaderFilePath, k_constraintGeometryShaderFuncName, k_constraintShaderFilePath, k_constraintPixelShaderFuncName, retrieveConstraintInputLayoutElementDesc(), k_constraintVertexDataLayoutDescCount }
{
	Storm::ConstantBufferHolder::initialize<ConstantBuffer>(device);
}

void Storm::ConstraintShader::setup(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera)
{
	// Setup the device context
	this->setupDeviceContext(deviceContext);

	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const Storm::SceneGraphicConfig &graphicConfig = configMgr.getSceneGraphicConfig();

	// Write shaders parameters
	{
		D3D11_MAPPED_SUBRESOURCE constraintsConstantBufferRessource;
		Storm::ResourceMapperGuard mapGuard{ deviceContext, _constantBuffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, constraintsConstantBufferRessource };

		ConstantBuffer*const resourceDataPtr = static_cast<ConstantBuffer*>(constraintsConstantBufferRessource.pData);

		resourceDataPtr->_viewMatrix = currentCamera.getTransposedViewMatrix();
		resourceDataPtr->_projMatrix = currentCamera.getTransposedProjectionMatrix();
		resourceDataPtr->_midThickness = graphicConfig._constraintThickness / 2.f;
		resourceDataPtr->_color.m128_f32[0] = graphicConfig._constraintColor.x();
		resourceDataPtr->_color.m128_f32[1] = graphicConfig._constraintColor.y();
		resourceDataPtr->_color.m128_f32[2] = graphicConfig._constraintColor.z();
		resourceDataPtr->_color.m128_f32[3] = graphicConfig._constraintColor.w();
	}

	ID3D11Buffer*const constantBufferTmp = _constantBuffer.Get();
	deviceContext->VSSetConstantBuffers(0, 1, &constantBufferTmp);
	deviceContext->GSSetConstantBuffers(0, 1, &constantBufferTmp);
	deviceContext->PSSetConstantBuffers(0, 1, &constantBufferTmp);
}
