cbuffer ConstantBuffer
{
	// ViewMatrix * ProjectionMatrix
	matrix _viewMatrix;
	matrix _projMatrix;

	float4 _color;
	float _midThickness;

	float _nearPlanePos;
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

	if (_nearPlanePos != -1.f)
	{
		output._position.z = _nearPlanePos;
	}

	return output;
}

[maxvertexcount(6)]
void particleForceGeometryShader(line GeometryInputType inputRaw[2], inout TriangleStream<PixelInputType> outputStream)
{
	GeometryInputType p0 = inputRaw[0];
	GeometryInputType p1 = inputRaw[1];

	float4 pos0 = mul(p0._position, _projMatrix);
	float4 pos1 = mul(p1._position, _projMatrix);

	const float4 lineVect = pos1 - pos0;

	// 90 degrees rotation of the force line
	float2 thicknessVect = float2(-lineVect.y, lineVect.x);

	const float thicknessNorm = length(thicknessVect);

	PixelInputType corner1;
	PixelInputType corner2;
	PixelInputType corner3;
	PixelInputType corner4;

	if (thicknessNorm > 0.00001f)
	{
		thicknessVect *= (_midThickness / thicknessNorm);

		corner1._position = float4(pos0.xy + thicknessVect, pos0.zw);
		corner2._position = float4(pos0.xy - thicknessVect, pos0.zw);
		corner3._position = float4(pos1.xy + thicknessVect, pos1.zw);
		corner4._position = float4(pos1.xy - thicknessVect, pos1.zw);
	}
	else
	{
		corner1._position = pos0;
		corner2._position = pos0;
		corner3._position = pos1;
		corner4._position = pos1;
	}

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
