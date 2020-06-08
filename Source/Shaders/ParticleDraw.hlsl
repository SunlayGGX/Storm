cbuffer ConstantBuffer
{
    // ViewMatrix * ProjectionMatrix
    matrix _viewProjMatrix;

    float _particleRadius;
};

struct VertexInputType
{
    float4 _position : POSITION;
    float4 _color : COLOR;
};

struct PixelInputType
{
    float4 _position : SV_POSITION;

    float4 _color : COLOR;

    // 1st elem is the distance to the center. The other are just junks ;)
    float4 _miscData : TEXCOORD0;
};


// Vertex Shader
PixelInputType particleVertexShader(VertexInputType input)
{
    PixelInputType output;

    // Change the position vector to be 4 units for proper matrix calculations.
    //output._position = input._position;
    output._position = mul(input._position, _viewProjMatrix);
    
    output._color = input._color;

    output._miscData.x = 0.f;

    return output;
}

// Pixel shader
float4 particlePixelShader(PixelInputType input) : SV_TARGET
{
    const float threshold = 1.f / sqrt(2);
    if (input._miscData.x < threshold)
    {
        return input._color;
    }

    return float4(0.f, 0.f, 0.f, 0.f);
}
