#include "VPShader.h"

#include "ShaderManager.h"



Storm::VPShaderBase::VPShaderBase(const ComPtr<ID3D11Device> &device, const std::string &vertexShaderFilePathStr, const std::string_view &vertexShaderFunctionName, const std::string &pixelShaderFilePathStr, const std::string_view &pixelShaderFunctionName, const D3D11_INPUT_ELEMENT_DESC* inputLayoutElemDesc, const unsigned int inputLayoutElemDescCount)
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
	pixelShaderBuffer.Attach(static_cast<ID3D10Blob*>(shaderMgr.requestCompiledShaderBlobs(pixelShaderFilePathStr, pixelShaderFunctionName, "ps_5_0")));

	const void*const vertexShaderBlobData = vertexShaderBuffer->GetBufferPointer();
	const std::size_t vertexShaderBufferSize = vertexShaderBuffer->GetBufferSize();

	Storm::throwIfFailed(device->CreateVertexShader(vertexShaderBlobData, vertexShaderBufferSize, nullptr, &_vertexShader));
	Storm::throwIfFailed(device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), nullptr, &_pixelShader));


	Storm::throwIfFailed(device->CreateInputLayout(inputLayoutElemDesc, inputLayoutElemDescCount, vertexShaderBlobData, vertexShaderBufferSize, &_vertexShaderInputLayout));
}

void Storm::VPShaderBase::setupDeviceContext(const ComPtr<ID3D11DeviceContext> &deviceContext) const
{
	deviceContext->IASetInputLayout(_vertexShaderInputLayout.Get());

	deviceContext->VSSetShader(_vertexShader.Get(), nullptr, 0);
	deviceContext->PSSetShader(_pixelShader.Get(), nullptr, 0);
}
