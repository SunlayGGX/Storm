cbuffer ConstantBuffer
{
	matrix _viewMatrix;
	matrix _projectionMatrix;

	float4 _areaColor;
};

struct VertexInputType
{
	float4 _position : POSITION;
};

struct PixelInputType
{
	float4 _position : SV_POSITION;
};

PixelInputType areaVertexShader(VertexInputType input)
{
	PixelInputType output;

	// Change the position vector to be 4 units for proper matrix calculations.
	input._position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output._position = mul(input._position, _viewMatrix);
	output._position = mul(output._position, _projectionMatrix);

	return output;
}

float4 areaPixelShader(PixelInputType input) : SV_TARGET
{
	return _areaColor;
}