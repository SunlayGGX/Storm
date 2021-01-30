cbuffer ConstantBuffer
{
	matrix _worldMatrix;
	matrix _viewProjMatrix;
	float4 _eyePosition;

	float4 _areaColor;
};

struct VertexInputType
{
	float4 _position : POSITION;
	float4 _normal : NORMAL;
};

struct PixelInputType
{
	float4 _position : SV_POSITION;
};

PixelInputType areaVertexShader(VertexInputType input)
{
	PixelInputType output;

	// We suppose we receive a vertex, not a vector. Therefore input._position.w should be 1.f.
	// input._position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output._position = mul(input._position, _worldMatrix);

	output._position = mul(output._position, _viewProjMatrix);

	return output;
}

float4 areaPixelShader(PixelInputType input) : SV_TARGET
{
	return _areaColor;
}