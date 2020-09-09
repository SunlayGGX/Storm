#include "ConstraintShader.h"


#include "ThrowIfFailed.h"
#include "MemoryHelper.h"


namespace
{
	static const std::string k_constraintShaderFilePath = "Shaders/ConstraintsDraw.hlsl";
	static constexpr std::string_view k_constraintVertexShaderFuncName = "constraintVertexShader";
	static constexpr std::string_view k_constraintGeometryShaderFuncName = "constraintGeometryShader";
	static constexpr std::string_view k_constraintPixelShaderFuncName = "constraintPixelShader";

	enum : unsigned int
	{
		k_constraintVertexDataLayoutDescCount = 1
	};

	inline D3D11_INPUT_ELEMENT_DESC* retrieveConstraintInputLayoutElementDesc()
	{
		static D3D11_INPUT_ELEMENT_DESC constraintVertexDataLayoutDesc[k_constraintVertexDataLayoutDescCount];

		return constraintVertexDataLayoutDesc;
	}
}


Storm::ConstraintShader::ConstraintShader(const ComPtr<ID3D11Device> &device) :
	Storm::VPShaderBase{ device, k_constraintShaderFilePath, k_constraintVertexShaderFuncName, k_constraintShaderFilePath, k_constraintGeometryShaderFuncName, k_constraintShaderFilePath, k_constraintPixelShaderFuncName, retrieveConstraintInputLayoutElementDesc(), k_constraintVertexDataLayoutDescCount }
{
	Storm::ConstantBufferHolder::initialize<ConstantBuffer>(device);
}

void Storm::ConstraintShader::setup(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera)
{
	
}
