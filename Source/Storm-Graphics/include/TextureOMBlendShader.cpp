#include "TextureOMBlendShader.h"

#include "ResourceMapperGuard.h"
#include "MemoryHelper.h"


namespace
{
	struct ConstantBuffer
	{

	};

	static const std::string k_blendShaderFilePath = "Shaders/OutputBlend.hlsl";
	static constexpr std::string_view k_blendVertexShaderFuncName = "blendVertexShader";
	static constexpr std::string_view k_blendPixelShaderFuncName = "blendPixelShader";

	enum : unsigned int
	{
		k_blendVertexDataLayoutDescCount = 1
	};

	inline D3D11_INPUT_ELEMENT_DESC* retrieveBlendInputLayoutElementDesc()
	{
		static D3D11_INPUT_ELEMENT_DESC blendVertexDataLayoutDesc[k_blendVertexDataLayoutDescCount];

		{
			D3D11_INPUT_ELEMENT_DESC &currentVertexDataLayoutDesc = blendVertexDataLayoutDesc[0];
			currentVertexDataLayoutDesc.SemanticName = "POSITION";
			currentVertexDataLayoutDesc.SemanticIndex = 0;
			currentVertexDataLayoutDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
			currentVertexDataLayoutDesc.InputSlot = 0;
			currentVertexDataLayoutDesc.AlignedByteOffset = 0;
			currentVertexDataLayoutDesc.InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
			currentVertexDataLayoutDesc.InstanceDataStepRate = 0;
		}

		return blendVertexDataLayoutDesc;
	}
}



Storm::TextureOMBlendShader::TextureOMBlendShader(const ComPtr<ID3D11Device> &device) :
	Storm::VPShaderBase{ device, k_blendShaderFilePath, k_blendVertexShaderFuncName, k_blendShaderFilePath, k_blendPixelShaderFuncName, retrieveBlendInputLayoutElementDesc(), k_blendVertexDataLayoutDescCount }
{
	//Storm::ConstantBufferHolder::initialize<ConstantBuffer>(device);
}

void Storm::TextureOMBlendShader::setup(const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &/*currentCamera*/, ID3D11ShaderResourceView* toBlend)
{
	this->setupDeviceContext(deviceContext);

	ID3D11Buffer*const constantBufferTmp = _constantBuffer.Get();
	deviceContext->VSSetConstantBuffers(0, 1, &constantBufferTmp);
	deviceContext->PSSetConstantBuffers(0, 1, &constantBufferTmp);

	deviceContext->PSSetShaderResources(0, 1, &toBlend);
}
