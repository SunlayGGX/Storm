cbuffer ConstantBuffer
{
    // ViewMatrix * ProjectionMatrix
    matrix _viewMatrix;
    matrix _projMatrix;

    float4 _color;
    float _midThickness;
};

struct VertexInputType
{
    float3 _position : POSITION;
};

struct GeometryInputType
{
    float4 _position : POSITION;
};

struct PixelInputType
{
    float4 _position : SV_POSITION;
};


// Vertex Shader
GeometryInputType particleForceVertexShader(VertexInputType input)
{
    GeometryInputType output;

    // Change the position vector to be 4 units for proper matrix calculations.
    //output._position = input._position;
    output._position = mul(float4(input._position, 1.f), _viewMatrix);

    return output;
}

[maxvertexcount(6)]
void particleForceGeometryShader(line GeometryInputType inputRaw[2], inout TriangleStream<PixelInputType> outputStream)
{
    GeometryInputType p0 = inputRaw[0];
    GeometryInputType p1 = inputRaw[1];

    const float4 pos0 = mul(p0._position, _projMatrix);
    const float4 pos1 = mul(p1._position, _projMatrix);

    PixelInputType corner1;
    corner1._position = float4(pos0.x + _midThickness, pos0.yzw);

    PixelInputType corner2;
    corner2._position = float4(pos0.x - _midThickness, pos0.yzw);

    PixelInputType corner3;
    corner3._position = float4(pos1.x + _midThickness, pos1.yzw);

    PixelInputType corner4;
    corner4._position = float4(pos1.x - _midThickness, pos1.yzw);

    outputStream.Append(corner2);
    outputStream.Append(corner1);
    outputStream.Append(corner3);
    outputStream.RestartStrip();

    outputStream.Append(corner2);
    outputStream.Append(corner3);
    outputStream.Append(corner4);
    outputStream.RestartStrip();
}

// Pixel shader
float4 particleForcePixelShader(PixelInputType input) : SV_TARGET
{
    return _color;
}
