cbuffer ConstantBuffer
{
	// ViewMatrix * ProjectionMatrix
	matrix _viewMatrix;
	matrix _projMatrix;

	float _screenSpaceXOffset;
	float _screenSpaceYOffset;

	// How long the axis vector will be in screen space coordinate when it is displayed at it peak norm value.
	float _maxAxisLengthScreenUnit;

	float _midThickness;
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
	GeometryInputType p1 = inputRaw[0];

	float4 axisVect = p1._vect;

	const float3 pos0 = float3(_screenSpaceXOffset, _screenSpaceYOffset, 0.0);

	float3 pos1 = mul(axisVect, _projMatrix);
	pos1.x += _screenSpaceXOffset;
	pos1.y += _screenSpaceYOffset;

	// To be displayed on HUD
	pos1.z = 0.0;

	float3 lineVect = pos1 - pos0;

	lineVect *= _maxAxisLengthScreenUnit;
	pos1 = pos0 + lineVect;

	float3 thicknessVect = cross(float3(0.f, 0.f, 1.f), lineVect);
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

		corner1._position = float4(pos0.x + xThickness, pos0.y + yThickness, pos0.z, 1.f);
		corner2._position = float4(pos0.x - xThickness, pos0.y - yThickness, pos0.z, 1.f);
		corner3._position = float4(pos1.x + xThickness, pos1.y + yThickness, pos1.z, 1.f);
		corner4._position = float4(pos1.x - xThickness, pos1.y - yThickness, pos1.z, 1.f);
	}
	else
	{
		corner1._position = float4(pos0, 1.f);
		corner2._position = float4(pos0, 1.f);
		corner3._position = float4(pos1, 1.f);
		corner4._position = float4(pos1, 1.f);
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
