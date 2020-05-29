cbuffer ConstantBuffer
{
    matrix _worldMatrix;
    matrix _viewMatrix;
    matrix _projectionMatrix;
	
	float4 _gridColor;
	
	float4 _padding1;
	float4 _padding2;
	float4 _padding3;
};

struct VertexInputType
{
    float4 position : POSITION;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
};

PixelInputType ColorVertexShader(VertexInputType input)
{
    PixelInputType output;
    
    // Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;
	
    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    return output;
}

float4 ColorPixelShader(PixelInputType input) : SV_TARGET
{
    return _gridColor;
}
