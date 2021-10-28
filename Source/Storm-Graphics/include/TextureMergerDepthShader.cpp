#include "TextureMergerDepthShader.h"

#include "MemoryHelper.h"

namespace
{
	struct ConstantBuffer
	{
		DirectX::XMMATRIX _viewProjMatrix;
	};

	static const std::string k_textureMergerDepthShaderFilePath = "Shaders/TextureMergerDepth.hlsl";
	static constexpr std::string_view k_textureMergerDepthVertexShaderFuncName = "textureMergerDepthVertexShader";
	static constexpr std::string_view k_textureMergerDepthPixelShaderFuncName = "textureMergerDepthPixelShader";

	enum : unsigned int
	{
		k_textureMergerDepthVertexDataLayoutDescCount = 1
	};

	inline D3D11_INPUT_ELEMENT_DESC* retrieveTextureMergerDepthInputLayoutElementDesc()
	{
		static D3D11_INPUT_ELEMENT_DESC textureMergerDepthVertexDataLayoutDesc[k_textureMergerDepthVertexDataLayoutDescCount];

		{
			D3D11_INPUT_ELEMENT_DESC &currentVertexDataLayoutDesc = textureMergerDepthVertexDataLayoutDesc[0];
			currentVertexDataLayoutDesc.SemanticName = "POSITION";
			currentVertexDataLayoutDesc.SemanticIndex = 0;
			currentVertexDataLayoutDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
			currentVertexDataLayoutDesc.InputSlot = 0;
			currentVertexDataLayoutDesc.AlignedByteOffset = 0;
			currentVertexDataLayoutDesc.InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
			currentVertexDataLayoutDesc.InstanceDataStepRate = 0;
		}

		return textureMergerDepthVertexDataLayoutDesc;
	}
}

Storm::TextureMergerDepthShader::TextureMergerDepthShader(const ComPtr<ID3D11Device> &device) :
	Storm::VPShaderBase{ device, k_textureMergerDepthShaderFilePath, k_textureMergerDepthVertexShaderFuncName, k_textureMergerDepthShaderFilePath, k_textureMergerDepthPixelShaderFuncName, retrieveTextureMergerDepthInputLayoutElementDesc(), k_textureMergerDepthVertexDataLayoutDescCount }
{
	Storm::ConstantBufferHolder::initialize<ConstantBuffer>(device);
}
