#pragma once

#include "IRenderedElement.h"


namespace Storm
{
	class VPShaderBase : public Storm::IRenderedElement
	{
	public:
		VPShaderBase(const ComPtr<ID3D11Device> &device, const std::string &vertexShaderFilePathStr, const std::string_view &vertexShaderFunctionName, const std::string &pixelShaderFilePathStr, const std::string_view &pixelShaderFunctionName, const D3D11_INPUT_ELEMENT_DESC* inputLayoutElemDesc, const unsigned int inputLayoutElemDescCount);
		virtual ~VPShaderBase() = default;

	protected:
		virtual void setupDeviceContext(const ComPtr<ID3D11DeviceContext> &deviceContext) const;

	protected:
		ComPtr<ID3D11InputLayout> _vertexShaderInputLayout;
		ComPtr<ID3D11VertexShader> _vertexShader;
		ComPtr<ID3D11PixelShader> _pixelShader; 
	};
}
