cbuffer ConstantBuffer
{
	// ViewMatrix * ProjectionMatrix
	matrix _viewProjMatrix;

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
GeometryInputType gravityArrowVertexShader(VertexInputType input)
{
	GeometryInputType output;

	// Change the position vector to be 4 units for proper matrix calculations.
	output._vect = mul(input._vect, _viewProjMatrix);
	output._color = input._color;

	return output;
}

// Geometry shader
[maxvertexcount(9)]
void gravityArrowGeometryShader(point GeometryInputType inputRaw[1], inout TriangleStream<PixelInputType> outputStream)
{
	const float3 pos0 = float3(_screenSpaceXOffset, _screenSpaceYOffset, 0.0);

	float3 gravityVect = inputRaw[0]._vect;
	gravityVect /= length(gravityVect);
	gravityVect.x += _screenSpaceXOffset;
	gravityVect.y += _screenSpaceYOffset;

	// To be displayed on HUD
	gravityVect.z = 0.0;

	float3 lineVect = gravityVect - pos0;

	lineVect *= _maxAxisLengthScreenUnit;

	// The body of the arrow should be 80% of the line. The remaining 20% will be the arrow head.
	gravityVect = pos0 + lineVect * 0.8f;

	float3 thicknessVect = cross(float3(0.f, 0.f, 1.f), lineVect);
	float thicknessNorm = length(thicknessVect);

	PixelInputType corner1;
	PixelInputType corner2;
	PixelInputType corner3;
	PixelInputType corner4;

	PixelInputType head1;
	PixelInputType head2;
	PixelInputType head3;

	if (thicknessNorm > 0.00001f)
	{
		thicknessVect /= thicknessNorm;

		float xThickness = thicknessVect.x * _midThickness;
		float yThickness = thicknessVect.y * _midThickness;

		corner1._position = float4(pos0.x + xThickness, pos0.y + yThickness, pos0.z, 1.f);
		corner2._position = float4(pos0.x - xThickness, pos0.y - yThickness, pos0.z, 1.f);
		corner3._position = float4(gravityVect.x + xThickness, gravityVect.y + yThickness, gravityVect.z, 1.f);
		corner4._position = float4(gravityVect.x - xThickness, gravityVect.y - yThickness, gravityVect.z, 1.f);

		float arrowHeadXThickness = xThickness * 1.5f;
		float arrowHeadYThickness = yThickness * 1.5f;

		head1._position = corner3._position;
		head1._position.x += arrowHeadXThickness;
		head1._position.y += arrowHeadYThickness;

		head2._position = corner4._position;
		head2._position.x -= arrowHeadXThickness;
		head2._position.y -= arrowHeadYThickness;

		head3._position = float4(pos0 + lineVect, 1.f);
	}
	else
	{
		corner1._position = float4(pos0, 1.f);
		corner2._position = float4(pos0, 1.f);
		corner3._position = float4(gravityVect, 1.f);
		corner4._position = float4(gravityVect, 1.f);

		head1._position = float4(gravityVect, 1.f);
		head2._position = float4(gravityVect, 1.f);
		head3._position = float4(gravityVect, 1.f);
	}

	corner1._color = inputRaw[0]._color;
	corner2._color = inputRaw[0]._color;
	corner3._color = inputRaw[0]._color;
	corner4._color = inputRaw[0]._color;

	head1._color = inputRaw[0]._color;
	head2._color = inputRaw[0]._color;
	head3._color = inputRaw[0]._color;

	outputStream.Append(corner2);
	outputStream.Append(corner1);
	outputStream.Append(corner3);
	outputStream.RestartStrip();

	outputStream.Append(corner2);
	outputStream.Append(corner3);
	outputStream.Append(corner4);
	outputStream.RestartStrip();

	// Now, draw the head
	outputStream.Append(head2);
	outputStream.Append(head1);
	outputStream.Append(head3);
	outputStream.RestartStrip();
}

// Pixel shader
float4 gravityArrowPixelShader(PixelInputType input) : SV_TARGET
{
	return float4(0.3, 0.3, 0.3, 1.0);
}
