#include "ParticleForceShader.h"

#include "Camera.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "GraphicData.h"

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

	static const std::string k_particleForceShaderFilePath = "Shaders/ParticleForceDraw.hlsl";
	static constexpr std::string_view k_particleForceVertexShaderFuncName = "particleForceVertexShader";
	static constexpr std::string_view k_particleForceGeometryShaderFuncName = "particleForceGeometryShader";
	static constexpr std::string_view k_particleForcePixelShaderFuncName = "particleForcePixelShader";

	enum : unsigned int
	{
		k_particleForceVertexDataLayoutDescCount = 1
	};

	inline D3D11_INPUT_ELEMENT_DESC* retrieveParticleForceInputLayoutElementDesc()
	{
		static D3D11_INPUT_ELEMENT_DESC particleForceVertexDataLayoutDesc[k_particleForceVertexDataLayoutDescCount];

		{
			D3D11_INPUT_ELEMENT_DESC &currentVertexDataLayoutDesc = particleForceVertexDataLayoutDesc[0];
			currentVertexDataLayoutDesc.SemanticName = "POSITION";
			currentVertexDataLayoutDesc.SemanticIndex = 0;
			currentVertexDataLayoutDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT;
			currentVertexDataLayoutDesc.InputSlot = 0;
			currentVertexDataLayoutDesc.AlignedByteOffset = 0;
			currentVertexDataLayoutDesc.InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
			currentVertexDataLayoutDesc.InstanceDataStepRate = 0;
		}

		return particleForceVertexDataLayoutDesc;
	}
}


Storm::ParticleForceShader::ParticleForceShader(const ComPtr<ID3D11Device> &device) :
	Storm::VPShaderBase{ device, k_particleForceShaderFilePath, k_particleForceVertexShaderFuncName, k_particleForceShaderFilePath, k_particleForceGeometryShaderFuncName, k_particleForceShaderFilePath, k_particleForcePixelShaderFuncName, retrieveParticleForceInputLayoutElementDesc(), k_particleForceVertexDataLayoutDescCount }
{
	Storm::ConstantBufferHolder::initialize<ConstantBuffer>(device);
}

void Storm::ParticleForceShader::setup(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera)
{
	// Setup the device context
	this->setupDeviceContext(deviceContext);

	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const Storm::GraphicData &graphicConfig = configMgr.getGraphicData();

	// Write shaders parameters
	{
		D3D11_MAPPED_SUBRESOURCE particleForcesConstantBufferRessource;
		Storm::ResourceMapperGuard mapGuard{ deviceContext, _constantBuffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, particleForcesConstantBufferRessource };

		ConstantBuffer*const ressourceDataPtr = static_cast<ConstantBuffer*>(particleForcesConstantBufferRessource.pData);

		ressourceDataPtr->_viewMatrix = currentCamera.getTransposedViewMatrix();
		ressourceDataPtr->_projMatrix = currentCamera.getTransposedProjectionMatrix();
		ressourceDataPtr->_midThickness = graphicConfig._forceThickness / 2.f;
		ressourceDataPtr->_color.m128_f32[0] = graphicConfig._forceColor.x();
		ressourceDataPtr->_color.m128_f32[1] = graphicConfig._forceColor.y();
		ressourceDataPtr->_color.m128_f32[2] = graphicConfig._forceColor.z();
		ressourceDataPtr->_color.m128_f32[3] = graphicConfig._forceColor.w();
	}

	ID3D11Buffer*const constantBufferTmp = _constantBuffer.Get();
	deviceContext->VSSetConstantBuffers(0, 1, &constantBufferTmp);
	deviceContext->GSSetConstantBuffers(0, 1, &constantBufferTmp);
	deviceContext->PSSetConstantBuffers(0, 1, &constantBufferTmp);
}
