#include "AreaShader.h"

#include "Camera.h"

#include "ShaderMacroItem.h"

#include "MemoryHelper.h"
#include "ResourceMapperGuard.h"

#include "XMStormHelpers.h"


namespace
{
	struct ConstantBuffer
	{
		DirectX::XMMATRIX _worldMatrix;
		DirectX::XMMATRIX _viewProjMatrix;

		DirectX::XMVECTOR _eyePosition;

		DirectX::XMVECTOR _color;
	};

	static const std::string k_areaShaderFilePath = "Shaders/AreaDraw.hlsl";
	static constexpr std::string_view k_areaVertexShaderFuncName = "areaVertexShader";
	static constexpr std::string_view k_areaPixelShaderFuncName = "areaPixelShader";

	enum : unsigned int
	{
		k_areaVertexDataLayoutDescCount = 2
	};

	inline D3D11_INPUT_ELEMENT_DESC* retrieveBlowerInputLayoutElementDesc()
	{
		static D3D11_INPUT_ELEMENT_DESC areaVertexDataLayoutDesc[k_areaVertexDataLayoutDescCount];

		{
			D3D11_INPUT_ELEMENT_DESC &currentVertexDataLayoutDesc = areaVertexDataLayoutDesc[0];
			currentVertexDataLayoutDesc.SemanticName = "POSITION";
			currentVertexDataLayoutDesc.SemanticIndex = 0;
			currentVertexDataLayoutDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
			currentVertexDataLayoutDesc.InputSlot = 0;
			currentVertexDataLayoutDesc.AlignedByteOffset = 0;
			currentVertexDataLayoutDesc.InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
			currentVertexDataLayoutDesc.InstanceDataStepRate = 0;
		}

		{
			D3D11_INPUT_ELEMENT_DESC &currentVertexDataLayoutDesc = areaVertexDataLayoutDesc[1];
			currentVertexDataLayoutDesc.SemanticName = "NORMAL";
			currentVertexDataLayoutDesc.SemanticIndex = 0;
			currentVertexDataLayoutDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
			currentVertexDataLayoutDesc.InputSlot = 0;
			currentVertexDataLayoutDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			currentVertexDataLayoutDesc.InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
			currentVertexDataLayoutDesc.InstanceDataStepRate = 0;
		}

		return areaVertexDataLayoutDesc;
	}

	Storm::ShaderMacroContainer convertMacros(const std::span<const std::string_view> macros)
	{
		Storm::ShaderMacroContainer result;

		const std::size_t macroCount = macros.size();
		if (macroCount > 0)
		{
			result.reserve(macroCount);
			for (const std::string_view &macro : macros)
			{
				result.addMacro(macro, true);
			}
		}

		return result;
	}
}

Storm::AreaShader::AreaShader(const ComPtr<ID3D11Device> &device, const uint32_t indexCount, const std::span<const std::string_view> macros) :
	Storm::VPShaderBase{ device, k_areaShaderFilePath, k_areaVertexShaderFuncName, k_areaShaderFilePath, k_areaPixelShaderFuncName, retrieveBlowerInputLayoutElementDesc(), k_areaVertexDataLayoutDescCount, convertMacros(macros) },
	_worldMat{ Storm::makeTransform(Storm::Vector3::Zero(), Storm::Quaternion::Identity()) } // By default, we won't use the world matrix.
{
	Storm::ConstantBufferHolder::initialize<ConstantBuffer>(device);
}

void Storm::AreaShader::setup(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera, const DirectX::XMVECTOR &color)
{
	this->setupDeviceContext(deviceContext);

	// Write shaders parameters
	{
		D3D11_MAPPED_SUBRESOURCE areaConstantBufferRessource;
		Storm::ResourceMapperGuard mapGuard{ deviceContext, _constantBuffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, areaConstantBufferRessource };

		ConstantBuffer*const ressourceDataPtr = static_cast<ConstantBuffer*>(areaConstantBufferRessource.pData);
		
		ressourceDataPtr->_worldMatrix = _worldMat;
		ressourceDataPtr->_viewProjMatrix = currentCamera.getTransposedViewProjMatrix();
		ressourceDataPtr->_eyePosition = DirectX::XMLoadFloat3(&currentCamera.getPosition());
		ressourceDataPtr->_color = color;
	}

	ID3D11Buffer*const constantBufferTmp = _constantBuffer.Get();
	deviceContext->VSSetConstantBuffers(0, 1, &constantBufferTmp);
	deviceContext->PSSetConstantBuffers(0, 1, &constantBufferTmp);
}

void Storm::AreaShader::updateWorldMat(const DirectX::XMVECTOR &position, const Storm::Quaternion &rotation, const Storm::Vector3 &scale)
{
	_worldMat = Storm::makeTransform(position, rotation, scale);
}
