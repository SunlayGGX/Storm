#pragma once


namespace Storm
{
	class VPShaderBase
	{
	public:
		VPShaderBase(const ComPtr<ID3D11Device> device, const std::string &vertexShaderFilePathStr, const std::string_view &vertexShaderFunctionName, const std::string &pixelShaderFilePathStr, const std::string_view &pixelShaderFunctionName);
		virtual ~VPShaderBase() = default;

	protected:
		ComPtr<ID3D11VertexShader> _vertexShader;
		ComPtr<ID3D11PixelShader> _pixelShader;
	};
}
