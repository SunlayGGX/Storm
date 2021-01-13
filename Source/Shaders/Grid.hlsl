cbuffer ConstantBuffer
{
	matrix _viewMatrix;
	matrix _projectionMatrix;

	float4 _gridColor;
};

struct VertexInputType
{
	float4 _position : POSITION;
};

struct PixelInputType
{
	float4 _position : SV_POSITION;
};

PixelInputType gridVertexShader(VertexInputType input)
{
	PixelInputType output;

	// Change the position vector to be 4 units for proper matrix calculations.
	input._position.w = 1.0f;

	// Calculate the position of the vertex against the view and projection matrices. (world always being identity since the grid don't move from its object space, we don't care about it).
	output._position = mul(input._position, _viewMatrix);
	output._position = mul(output._position, _projectionMatrix);

	return output;
}

float4 gridPixelShader(PixelInputType input) : SV_TARGET
{
	return _gridColor;
}
