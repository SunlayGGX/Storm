cbuffer ConstantBuffer
{
    // ViewMatrix * ProjectionMatrix
    matrix _viewMatrix;
    matrix _projMatrix;

    float _midThickness;
    float4 _color;
};

struct VertexInputType
{
    float4 _position : POSITION;
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
GeometryInputType constraintVertexShader(VertexInputType input)
{
    GeometryInputType output;

    // Change the position vector to be 4 units for proper matrix calculations.
    //output._position = input._position;
    output._position = mul(input._position, _viewMatrix);

    return output;
}

[maxvertexcount(6)]
void constraintGeometryShader(line GeometryInputType inputRaw[2], inout TriangleStream<PixelInputType> outputStream)
{
    GeometryInputType p0 = inputRaw[0];
    GeometryInputType p1 = inputRaw[1];

    PixelInputType corner1;
    corner1._position = float4(p0._position.x + _midThickness, p0._position.yzw);
    corner1._position = mul(corner1._position, _projMatrix);

    PixelInputType corner2;
    corner2._position = float4(p0._position.x - _midThickness, p0._position.yzw);
    corner2._position = mul(corner2._position, _projMatrix);

    PixelInputType corner3;
    corner3._position = float4(p1._position.x + _midThickness, p1._position.yzw);
    corner3._position = mul(corner3._position, _projMatrix);

    PixelInputType corner4;
    corner4._position = float4(p1._position.x - _midThickness, p1._position.yzw);
    corner4._position = mul(corner4._position, _projMatrix);

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
float4 constraintPixelShader(PixelInputType input) : SV_TARGET
{
    return _color;
}
