#include "ParticleShader.h"

#include "MemoryHelper.h"


namespace
{
	struct ConstantBuffer
	{
		DirectX::XMMATRIX _viewProjMatrix;
		float _pointSize;
	};

	static const std::string k_particleShaderFilePath = "Shaders/ParticleDraw.hlsl";
	static constexpr std::string_view k_particleVertexShaderFuncName = "particleVertexShader";
	static constexpr std::string_view k_particlePixelShaderFuncName = "particlePixelShader";

	enum : unsigned int
	{
		k_particleVertexDataLayoutDescCount = 2
	};

	inline D3D11_INPUT_ELEMENT_DESC* retrieveMeshInputLayoutElementDesc()
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
			currentVertexDataLayoutDesc.SemanticName = "PSIZE";
			currentVertexDataLayoutDesc.SemanticIndex = 0;
			currentVertexDataLayoutDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
			currentVertexDataLayoutDesc.InputSlot = 0;
			currentVertexDataLayoutDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			currentVertexDataLayoutDesc.InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
			currentVertexDataLayoutDesc.InstanceDataStepRate = 0;
		}

		return particleVertexDataLayoutDesc;
	}
}


Storm::ParticleShader::ParticleShader(const ComPtr<ID3D11Device> &device) :
	Storm::VPShaderBase{ device, k_particleShaderFilePath, k_particleVertexShaderFuncName, k_particleShaderFilePath, k_particlePixelShaderFuncName, retrieveMeshInputLayoutElementDesc(), k_particleVertexDataLayoutDescCount }
{
	Storm::ConstantBufferHolder::initialize<ConstantBuffer>(device);
}

void Storm::ParticleShader::setup(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera)
{

}
