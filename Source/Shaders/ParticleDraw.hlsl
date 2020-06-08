cbuffer ConstantBuffer
{
    // ViewMatrix * ProjectionMatrix
    matrix _viewProjMatrix;

    float _particleRadius;
};

struct VertexInputType
{
    float4 _position : POSITION;
    float _ptSize : PSIZE;
};

struct PixelInputType
{
    float4 _position : SV_POSITION;

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

    output._miscData.x = input._ptSize;

    return output;
}

// Pixel shader
float4 particlePixelShader(PixelInputType input) : SV_TARGET
{
    return float4(1.f, 1.f, 1.f, 1.f);
}
