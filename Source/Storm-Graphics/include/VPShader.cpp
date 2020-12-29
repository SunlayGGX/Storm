#include "VPShader.h"

#include "ShaderManager.h"



Storm::VPShaderBase::VPShaderBase(const ComPtr<ID3D11Device> &device, const std::string &vertexShaderFilePathStr, const std::string_view &vertexShaderFunctionName, const std::string &pixelShaderFilePathStr, const std::string_view &pixelShaderFunctionName, const D3D11_INPUT_ELEMENT_DESC* inputLayoutElemDesc, const unsigned int inputLayoutElemDescCount) :
	_geometryShader{ nullptr }
{
	const std::filesystem::path vertexShaderFilePath{ vertexShaderFilePathStr };
	if (!std::filesystem::is_regular_file(vertexShaderFilePath))
	{
		Storm::throwException<Storm::Exception>(vertexShaderFilePathStr + " doesn't exist or cannot be a vertex shader!");
	}

	const std::filesystem::path pixelShaderFilePath{ pixelShaderFilePathStr };
	if (!std::filesystem::is_regular_file(pixelShaderFilePath))
	{
		Storm::throwException<Storm::Exception>(pixelShaderFilePathStr + " doesn't exist or cannot be a pixel shader!");
	}

	if (vertexShaderFunctionName.empty() || pixelShaderFunctionName.empty())
	{
		Storm::throwException<Storm::Exception>("Shaders function name shouldn't be empty!");
	}

	Storm::ShaderManager &shaderMgr = Storm::ShaderManager::instance();

	ComPtr<ID3DBlob> vertexShaderBuffer;
	vertexShaderBuffer.Attach(static_cast<ID3DBlob*>(shaderMgr.requestCompiledShaderBlobs(vertexShaderFilePathStr, vertexShaderFunctionName, "vs_5_0")));

	ComPtr<ID3DBlob> pixelShaderBuffer;
	pixelShaderBuffer.Attach(static_cast<ID3DBlob*>(shaderMgr.requestCompiledShaderBlobs(pixelShaderFilePathStr, pixelShaderFunctionName, "ps_5_0")));

	const void*const vertexShaderBlobData = vertexShaderBuffer->GetBufferPointer();
	const std::size_t vertexShaderBufferSize = vertexShaderBuffer->GetBufferSize();

	Storm::throwIfFailed(device->CreateVertexShader(vertexShaderBlobData, vertexShaderBufferSize, nullptr, &_vertexShader));
	Storm::throwIfFailed(device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), nullptr, &_pixelShader));

	Storm::throwIfFailed(device->CreateInputLayout(inputLayoutElemDesc, inputLayoutElemDescCount, vertexShaderBlobData, vertexShaderBufferSize, &_vertexShaderInputLayout));
}

Storm::VPShaderBase::VPShaderBase(const ComPtr<ID3D11Device> &device, const std::string &vertexShaderFilePathStr, const std::string_view &vertexShaderFunctionName, const std::string &geometryShaderFilePathStr, const std::string_view &geometryShaderFunctionName, const std::string &pixelShaderFilePathStr, const std::string_view &pixelShaderFunctionName, const D3D11_INPUT_ELEMENT_DESC* inputLayoutElemDesc, const unsigned int inputLayoutElemDescCount)
{
	const std::filesystem::path vertexShaderFilePath{ vertexShaderFilePathStr };
	if (!std::filesystem::is_regular_file(vertexShaderFilePath))
	{
		Storm::throwException<Storm::Exception>(vertexShaderFilePathStr + " doesn't exist or cannot be a vertex shader!");
	}

	const std::filesystem::path geometryShaderFilePath{ geometryShaderFilePathStr };
	if (!std::filesystem::is_regular_file(geometryShaderFilePath))
	{
		Storm::throwException<Storm::Exception>(geometryShaderFilePathStr + " doesn't exist or cannot be a geometry shader!");
	}

	const std::filesystem::path pixelShaderFilePath{ pixelShaderFilePathStr };
	if (!std::filesystem::is_regular_file(pixelShaderFilePath))
	{
		Storm::throwException<Storm::Exception>(pixelShaderFilePathStr + " doesn't exist or cannot be a pixel shader!");
	}

	if (vertexShaderFunctionName.empty() || pixelShaderFunctionName.empty() || geometryShaderFunctionName.empty())
	{
		Storm::throwException<Storm::Exception>("Shaders function name shouldn't be empty!");
	}

	Storm::ShaderManager &shaderMgr = Storm::ShaderManager::instance();

	ComPtr<ID3DBlob> vertexShaderBuffer;
	vertexShaderBuffer.Attach(static_cast<ID3DBlob*>(shaderMgr.requestCompiledShaderBlobs(vertexShaderFilePathStr, vertexShaderFunctionName, "vs_5_0")));

	ComPtr<ID3DBlob> geometryShaderBuffer;
	geometryShaderBuffer.Attach(static_cast<ID3DBlob*>(shaderMgr.requestCompiledShaderBlobs(geometryShaderFilePathStr, geometryShaderFunctionName, "gs_5_0")));

	ComPtr<ID3DBlob> pixelShaderBuffer;
	pixelShaderBuffer.Attach(static_cast<ID3DBlob*>(shaderMgr.requestCompiledShaderBlobs(pixelShaderFilePathStr, pixelShaderFunctionName, "ps_5_0")));

	const void*const vertexShaderBlobData = vertexShaderBuffer->GetBufferPointer();
	const std::size_t vertexShaderBufferSize = vertexShaderBuffer->GetBufferSize();

	Storm::throwIfFailed(device->CreateVertexShader(vertexShaderBlobData, vertexShaderBufferSize, nullptr, &_vertexShader));
	Storm::throwIfFailed(device->CreateGeometryShader(geometryShaderBuffer->GetBufferPointer(), geometryShaderBuffer->GetBufferSize(), nullptr, &_geometryShader));
	Storm::throwIfFailed(device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), nullptr, &_pixelShader));

	Storm::throwIfFailed(device->CreateInputLayout(inputLayoutElemDesc, inputLayoutElemDescCount, vertexShaderBlobData, vertexShaderBufferSize, &_vertexShaderInputLayout));
}

Storm::VPShaderBase::VPShaderBase(const ComPtr<ID3D11Device> &device, const std::string &vertexShaderFilePathStr, const std::string_view &vertexShaderFunctionName, const std::string &pixelShaderFilePathStr, const std::string_view &pixelShaderFunctionName, const D3D11_INPUT_ELEMENT_DESC* inputLayoutElemDesc, const unsigned int inputLayoutElemDescCount, const Storm::ShaderMacroContainer &shaderMacros)
{
	const std::filesystem::path vertexShaderFilePath{ vertexShaderFilePathStr };
	if (!std::filesystem::is_regular_file(vertexShaderFilePath))
	{
		Storm::throwException<Storm::Exception>(vertexShaderFilePathStr + " doesn't exist or cannot be a vertex shader!");
	}

	const std::filesystem::path pixelShaderFilePath{ pixelShaderFilePathStr };
	if (!std::filesystem::is_regular_file(pixelShaderFilePath))
	{
		Storm::throwException<Storm::Exception>(pixelShaderFilePathStr + " doesn't exist or cannot be a pixel shader!");
	}

	if (vertexShaderFunctionName.empty() || pixelShaderFunctionName.empty())
	{
		Storm::throwException<Storm::Exception>("Shaders function name shouldn't be empty!");
	}

	Storm::ShaderManager &shaderMgr = Storm::ShaderManager::instance();

	ComPtr<ID3DBlob> vertexShaderBuffer;
	vertexShaderBuffer.Attach(static_cast<ID3DBlob*>(shaderMgr.requestCompiledShaderBlobs(vertexShaderFilePathStr, vertexShaderFunctionName, "vs_5_0", shaderMacros)));

	ComPtr<ID3DBlob> pixelShaderBuffer;
	pixelShaderBuffer.Attach(static_cast<ID3DBlob*>(shaderMgr.requestCompiledShaderBlobs(pixelShaderFilePathStr, pixelShaderFunctionName, "ps_5_0", shaderMacros)));

	const void*const vertexShaderBlobData = vertexShaderBuffer->GetBufferPointer();
	const std::size_t vertexShaderBufferSize = vertexShaderBuffer->GetBufferSize();

	Storm::throwIfFailed(device->CreateVertexShader(vertexShaderBlobData, vertexShaderBufferSize, nullptr, &_vertexShader));
	Storm::throwIfFailed(device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), nullptr, &_pixelShader));

	Storm::throwIfFailed(device->CreateInputLayout(inputLayoutElemDesc, inputLayoutElemDescCount, vertexShaderBlobData, vertexShaderBufferSize, &_vertexShaderInputLayout));
}

Storm::VPShaderBase::VPShaderBase(const ComPtr<ID3D11Device> &device, const std::string &vertexShaderFilePathStr, const std::string_view &vertexShaderFunctionName, const std::string &geometryShaderFilePathStr, const std::string_view &geometryShaderFunctionName, const std::string &pixelShaderFilePathStr, const std::string_view &pixelShaderFunctionName, const D3D11_INPUT_ELEMENT_DESC* inputLayoutElemDesc, const unsigned int inputLayoutElemDescCount, const Storm::ShaderMacroContainer &shaderMacros)
{
	const std::filesystem::path vertexShaderFilePath{ vertexShaderFilePathStr };
	if (!std::filesystem::is_regular_file(vertexShaderFilePath))
	{
		Storm::throwException<Storm::Exception>(vertexShaderFilePathStr + " doesn't exist or cannot be a vertex shader!");
	}

	const std::filesystem::path geometryShaderFilePath{ geometryShaderFilePathStr };
	if (!std::filesystem::is_regular_file(geometryShaderFilePath))
	{
		Storm::throwException<Storm::Exception>(geometryShaderFilePathStr + " doesn't exist or cannot be a geometry shader!");
	}

	const std::filesystem::path pixelShaderFilePath{ pixelShaderFilePathStr };
	if (!std::filesystem::is_regular_file(pixelShaderFilePath))
	{
		Storm::throwException<Storm::Exception>(pixelShaderFilePathStr + " doesn't exist or cannot be a pixel shader!");
	}

	if (vertexShaderFunctionName.empty() || pixelShaderFunctionName.empty() || geometryShaderFunctionName.empty())
	{
		Storm::throwException<Storm::Exception>("Shaders function name shouldn't be empty!");
	}

	Storm::ShaderManager &shaderMgr = Storm::ShaderManager::instance();

	ComPtr<ID3DBlob> vertexShaderBuffer;
	vertexShaderBuffer.Attach(static_cast<ID3DBlob*>(shaderMgr.requestCompiledShaderBlobs(vertexShaderFilePathStr, vertexShaderFunctionName, "vs_5_0", shaderMacros)));

	ComPtr<ID3DBlob> geometryShaderBuffer;
	geometryShaderBuffer.Attach(static_cast<ID3DBlob*>(shaderMgr.requestCompiledShaderBlobs(geometryShaderFilePathStr, geometryShaderFunctionName, "gs_5_0", shaderMacros)));

	ComPtr<ID3DBlob> pixelShaderBuffer;
	pixelShaderBuffer.Attach(static_cast<ID3DBlob*>(shaderMgr.requestCompiledShaderBlobs(pixelShaderFilePathStr, pixelShaderFunctionName, "ps_5_0", shaderMacros)));

	const void*const vertexShaderBlobData = vertexShaderBuffer->GetBufferPointer();
	const std::size_t vertexShaderBufferSize = vertexShaderBuffer->GetBufferSize();

	Storm::throwIfFailed(device->CreateVertexShader(vertexShaderBlobData, vertexShaderBufferSize, nullptr, &_vertexShader));
	Storm::throwIfFailed(device->CreateGeometryShader(geometryShaderBuffer->GetBufferPointer(), geometryShaderBuffer->GetBufferSize(), nullptr, &_geometryShader));
	Storm::throwIfFailed(device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), nullptr, &_pixelShader));

	Storm::throwIfFailed(device->CreateInputLayout(inputLayoutElemDesc, inputLayoutElemDescCount, vertexShaderBlobData, vertexShaderBufferSize, &_vertexShaderInputLayout));
}

void Storm::VPShaderBase::setupDeviceContext(const ComPtr<ID3D11DeviceContext> &deviceContext) const
{
	deviceContext->VSSetShader(_vertexShader.Get(), nullptr, 0);

	if (_geometryShader)
	{
		deviceContext->GSSetShader(_geometryShader.Get(), nullptr, 0);
	}
	else
	{
		deviceContext->GSSetShader(nullptr, nullptr, 0);
	}

	deviceContext->IASetInputLayout(_vertexShaderInputLayout.Get());

	deviceContext->PSSetShader(_pixelShader.Get(), nullptr, 0);
}

void Storm::VPShaderBase::draw(unsigned int indexCount, const ComPtr<ID3D11DeviceContext> &deviceContext) const
{
	deviceContext->DrawIndexed(indexCount, 0, 0);
}
