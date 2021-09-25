#include "NormalsShader.h"

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

	// We use the same shader that particle force draw except for a macro that remove the always on top feature.
	static const std::string k_normalsShaderFilePath = "Shaders/ParticleForceDraw.hlsl";
	static constexpr std::string_view k_normalsVertexShaderFuncName = "particleForceVertexShader";
	static constexpr std::string_view k_normalsGeometryShaderFuncName = "particleForceGeometryShader";
	static constexpr std::string_view k_normalsPixelShaderFuncName = "particleForcePixelShader";

	enum : unsigned int
	{
		k_normalsVertexDataLayoutDescCount = 1
	};

	inline D3D11_INPUT_ELEMENT_DESC* retrieveNormalsInputLayoutElementDesc()
	{
		static D3D11_INPUT_ELEMENT_DESC normalsVertexDataLayoutDesc[k_normalsVertexDataLayoutDescCount];

		{
			D3D11_INPUT_ELEMENT_DESC &currentVertexDataLayoutDesc = normalsVertexDataLayoutDesc[0];
			currentVertexDataLayoutDesc.SemanticName = "POSITION";
			currentVertexDataLayoutDesc.SemanticIndex = 0;
			currentVertexDataLayoutDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT;
			currentVertexDataLayoutDesc.InputSlot = 0;
			currentVertexDataLayoutDesc.AlignedByteOffset = 0;
			currentVertexDataLayoutDesc.InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
			currentVertexDataLayoutDesc.InstanceDataStepRate = 0;
		}

		return normalsVertexDataLayoutDesc;
	}
}


Storm::NormalsShader::NormalsShader(const ComPtr<ID3D11Device> &device) :
	Storm::VPShaderBase{ device, k_normalsShaderFilePath, k_normalsVertexShaderFuncName, k_normalsShaderFilePath, k_normalsGeometryShaderFuncName, k_normalsShaderFilePath, k_normalsPixelShaderFuncName, retrieveNormalsInputLayoutElementDesc(), k_normalsVertexDataLayoutDescCount }
{
	Storm::ConstantBufferHolder::initialize<ConstantBuffer>(device);
}

void Storm::NormalsShader::setup(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera)
{
	// Setup the device context
	this->setupDeviceContext(deviceContext);

	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const Storm::SceneGraphicConfig &graphicConfig = configMgr.getSceneGraphicConfig();

	// Write shaders parameters
	{
		D3D11_MAPPED_SUBRESOURCE normalsConstantBufferRessource;
		Storm::ResourceMapperGuard mapGuard{ deviceContext, _constantBuffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, normalsConstantBufferRessource };

		ConstantBuffer*const ressourceDataPtr = static_cast<ConstantBuffer*>(normalsConstantBufferRessource.pData);
		
		ressourceDataPtr->_viewMatrix = currentCamera.getTransposedViewMatrix();
		ressourceDataPtr->_projMatrix = currentCamera.getTransposedProjectionMatrix();
		ressourceDataPtr->_midThickness = graphicConfig._forceThickness / 2.f;
		ressourceDataPtr->_color.m128_f32[0] = graphicConfig._normalsColor.x();
		ressourceDataPtr->_color.m128_f32[1] = graphicConfig._normalsColor.y();
		ressourceDataPtr->_color.m128_f32[2] = graphicConfig._normalsColor.z();
		ressourceDataPtr->_color.m128_f32[3] = graphicConfig._normalsColor.w();
	}

	ID3D11Buffer*const constantBufferTmp = _constantBuffer.Get();
	deviceContext->VSSetConstantBuffers(0, 1, &constantBufferTmp);
	deviceContext->GSSetConstantBuffers(0, 1, &constantBufferTmp);
	deviceContext->PSSetConstantBuffers(0, 1, &constantBufferTmp);
}
