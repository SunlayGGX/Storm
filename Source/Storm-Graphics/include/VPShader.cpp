#include "VPShader.h"

#include "ShaderManager.h"

#include "ThrowException.h"

#include <comdef.h>



Storm::VPShaderBase::VPShaderBase(const ComPtr<ID3D11Device> device, const std::string &vertexShaderFilePathStr, const std::string_view &vertexShaderFunctionName, const std::string &pixelShaderFilePathStr, const std::string_view &pixelShaderFunctionName)
{
	const std::filesystem::path vertexShaderFilePath{ vertexShaderFilePathStr };
	if (!std::filesystem::is_regular_file(vertexShaderFilePath))
	{
		Storm::throwException<std::exception>(vertexShaderFilePathStr + " doesn't exist or cannot be a vertex shader!");
	}

	const std::filesystem::path pixelShaderFilePath{ pixelShaderFilePathStr };
	if (!std::filesystem::is_regular_file(pixelShaderFilePath))
	{
		Storm::throwException<std::exception>(pixelShaderFilePathStr + " doesn't exist or cannot be a pixel shader!");
	}

	if (vertexShaderFunctionName.empty() || pixelShaderFunctionName.empty())
	{
		Storm::throwException<std::exception>("Shaders function name shouldn't be empty!");
	}

	Storm::ShaderManager &shaderMgr = Storm::ShaderManager::instance();

	ComPtr<ID3D10Blob> vertexShaderBuffer;
	vertexShaderBuffer.Attach(static_cast<ID3D10Blob*>(shaderMgr.requestCompiledShaderBlobs(vertexShaderFilePathStr, vertexShaderFunctionName, "vs_5_0")));

	ComPtr<ID3D10Blob> pixelShaderBuffer;
	pixelShaderBuffer.Attach(static_cast<ID3D10Blob*>(shaderMgr.requestCompiledShaderBlobs(pixelShaderFilePathStr, pixelShaderFunctionName, "vs_5_0")));

	HRESULT res = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), nullptr, &_vertexShader);
	if (FAILED(res))
	{
		Storm::throwException<std::exception>("Vertex shaders creation failed! " + std::filesystem::path{ _com_error{ res }.ErrorMessage() }.string());
	}

	res = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), nullptr, &_pixelShader);
	if (FAILED(res))
	{
		Storm::throwException<std::exception>("Pixel shaders creation failed! " + std::filesystem::path{ _com_error{ res }.ErrorMessage() }.string());
	}
}
