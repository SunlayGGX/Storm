cbuffer ConstantBuffer
{
    // ViewMatrix * ProjectionMatrix
    matrix _viewMatrix;
    matrix _projMatrix;
	
	float3 _originPosition;
	
    float _midThickness;
	float _nearPlanePos;
};

struct VertexInputType
{
    float4 _vect : POSITION;
	float4 _color : COLOR;
};

struct GeometryInputType
{
    float4 _vect : POSITION;
	float4 _color : COLOR;
};

struct PixelInputType
{
    float4 _position : SV_POSITION;
	float4 _color : COLOR;
};


// Vertex Shader
GeometryInputType coordinateSystemVertexShader(VertexInputType input)
{
    GeometryInputType output;

    // Change the position vector to be 4 units for proper matrix calculations.
    output._vect = mul(input._vect, _viewMatrix);
	output._color = input._color;

    return output;
}

[maxvertexcount(6)]
void coordinateSystemGeometryShader(point GeometryInputType inputRaw[1], inout TriangleStream<PixelInputType> outputStream)
{
	GeometryInputType p0;
    p0._vect = mul(float4(_originPosition, 1.f), _viewMatrix);
	
    GeometryInputType p1 = inputRaw[0];

    const float4 pos0 = mul(p0._vect, _projMatrix);
    const float4 pos1 = mul(p1._vect + p0._vect, _projMatrix);

    float4 lineVect = pos1 - pos0;
    float3 thicknessVect = cross(float3(0.f, 0.f, 1.f), lineVect.xyz);

    float thicknessNorm = length(thicknessVect);

    PixelInputType corner1;
    PixelInputType corner2;
    PixelInputType corner3;
    PixelInputType corner4;

    if (thicknessNorm > 0.00001f)
    {
        thicknessVect /= thicknessNorm;

        float xThickness = thicknessVect.x * _midThickness;
        float yThickness = thicknessVect.y * _midThickness;

        corner1._position = float4(pos0.x + xThickness, pos0.y + yThickness, pos0.zw);
        corner2._position = float4(pos0.x - xThickness, pos0.y - yThickness, pos0.zw);
        corner3._position = float4(pos1.x + xThickness, pos1.y + yThickness, pos1.zw);
        corner4._position = float4(pos1.x - xThickness, pos1.y - yThickness, pos1.zw);
    }
    else
    {
        corner1._position = pos0;
        corner2._position = pos0;
        corner3._position = pos1;
        corner4._position = pos1;
    }
	
	corner1._color = p1._color;
	corner2._color = p1._color;
	corner3._color = p1._color;
	corner4._color = p1._color;

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
float4 coordinateSystemPixelShader(PixelInputType input) : SV_TARGET
{
    return input._color;
}
