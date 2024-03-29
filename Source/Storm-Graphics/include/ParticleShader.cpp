#include "ParticleShader.h"
#include "Camera.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "GeneralGraphicConfig.h"
#include "SceneSimulationConfig.h"

#include "ShaderMacroItem.h"

#include "MemoryHelper.h"
#include "ResourceMapperGuard.h"


namespace
{
	struct ConstantBuffer
	{
		DirectX::XMMATRIX _viewMatrix;
		DirectX::XMMATRIX _projMatrix;
		float _pointSize;
		float _nearPlaneDist;
	};

	static const std::string k_particleShaderFilePath = "Shaders/ParticleDraw.hlsl";
	static constexpr std::string_view k_particleVertexShaderFuncName = "particleVertexShader";
	static constexpr std::string_view k_particleGeometryShaderFuncName = "particleGeometryShader";
	static constexpr std::string_view k_particlePixelShaderFuncName = "particlePixelShader";

	enum : unsigned int
	{
		k_particleVertexDataLayoutDescCount = 2
	};

	inline D3D11_INPUT_ELEMENT_DESC* retrieveParticleInputLayoutElementDesc()
	{
		static D3D11_INPUT_ELEMENT_DESC particleVertexDataLayoutDesc[k_particleVertexDataLayoutDescCount];

		{
			D3D11_INPUT_ELEMENT_DESC &currentVertexDataLayoutDesc = particleVertexDataLayoutDesc[0];
			currentVertexDataLayoutDesc.SemanticName = "POSITION";
			currentVertexDataLayoutDesc.SemanticIndex = 0;
			currentVertexDataLayoutDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
			currentVertexDataLayoutDesc.InputSlot = 0;
			currentVertexDataLayoutDesc.AlignedByteOffset = 0;
			currentVertexDataLayoutDesc.InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
			currentVertexDataLayoutDesc.InstanceDataStepRate = 0;
		}
		{
			D3D11_INPUT_ELEMENT_DESC &currentVertexDataLayoutDesc = particleVertexDataLayoutDesc[1];
			currentVertexDataLayoutDesc.SemanticName = "COLOR";
			currentVertexDataLayoutDesc.SemanticIndex = 0;
			currentVertexDataLayoutDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
			currentVertexDataLayoutDesc.InputSlot = 0;
			currentVertexDataLayoutDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			currentVertexDataLayoutDesc.InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
			currentVertexDataLayoutDesc.InstanceDataStepRate = 0;
		}

		return particleVertexDataLayoutDesc;
	}

	Storm::ShaderMacroContainer retrieveParticleShaderMacro()
	{
		const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();

		return Storm::ShaderMacroContainer{}
			.addMacro("STORM_SELECTED_PARTICLE_ON_TOP", configMgr.getGeneralGraphicConfig()._selectedParticleShouldBeTopMost)
			;
	}
}


Storm::ParticleShader::ParticleShader(const ComPtr<ID3D11Device> &device) :
	Storm::VPShaderBase{ device, k_particleShaderFilePath, k_particleVertexShaderFuncName, k_particleShaderFilePath, k_particleGeometryShaderFuncName, k_particleShaderFilePath, k_particlePixelShaderFuncName, retrieveParticleInputLayoutElementDesc(), k_particleVertexDataLayoutDescCount, retrieveParticleShaderMacro() }
{
	Storm::ConstantBufferHolder::initialize<ConstantBuffer>(device);
}

void Storm::ParticleShader::setup(const ComPtr<ID3D11Device> &/*device*/, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera, const bool firstPass)
{
	// Setup the device context
	this->setupDeviceContext(deviceContext);

	// Write shaders parameters
	{
		D3D11_MAPPED_SUBRESOURCE particleConstantBufferRessource;
		Storm::ResourceMapperGuard mapGuard{ deviceContext, _constantBuffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, particleConstantBufferRessource };

		ConstantBuffer*const ressourceDataPtr = static_cast<ConstantBuffer*>(particleConstantBufferRessource.pData);

		ressourceDataPtr->_viewMatrix = currentCamera.getTransposedViewMatrix();
		ressourceDataPtr->_projMatrix = currentCamera.getTransposedProjectionMatrix();
		ressourceDataPtr->_pointSize = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>().getSceneSimulationConfig()._particleRadius;
		ressourceDataPtr->_nearPlaneDist = firstPass ? currentCamera.getNearPlane() : -100.f;
	}

	ID3D11Buffer*const constantBufferTmp = _constantBuffer.Get();
	deviceContext->VSSetConstantBuffers(0, 1, &constantBufferTmp);
	deviceContext->GSSetConstantBuffers(0, 1, &constantBufferTmp);
	deviceContext->PSSetConstantBuffers(0, 1, &constantBufferTmp);
}
