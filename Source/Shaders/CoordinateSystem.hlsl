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
GeometryInputType coordinateSystemVertexShader(VertexInputType input)
{
	GeometryInputType output;

	// Change the position vector to be 4 units for proper matrix calculations.
	output._vect = mul(input._vect, _viewProjMatrix);
	output._color = input._color;

	return output;
}

// Geometry shader
void produceAxisArrow(in GeometryInputType p1, inout TriangleStream<PixelInputType> outputStream)
{
	const float3 pos0 = float3(_screenSpaceXOffset, _screenSpaceYOffset, 0.0);

	float3 pos1 = p1._vect;
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

[maxvertexcount(18)]
void coordinateSystemGeometryShader(triangle GeometryInputType inputRaw[3], inout TriangleStream<PixelInputType> outputStream)
{
	uint handledInOrder[3];

	const float depth0 = inputRaw[0]._vect.z;
	const float depth1 = inputRaw[1]._vect.z;
	const float depth2 = inputRaw[2]._vect.z;

	// Here the axis should be drawn in the decrease depth order
	// (manually make the Painter's algorithm because everything will be flatten on HUD, and GPU pipeline will lose the info on what pixel to display over another pixel)
	if (depth0 > depth1)
	{
		if (depth0 > depth2)
		{
			handledInOrder[0] = 0;

			if (depth1 > depth2)
			{
				handledInOrder[1] = 1;
				handledInOrder[2] = 2;
			}
			else
			{
				handledInOrder[1] = 2;
				handledInOrder[2] = 1;
			}
		}
		else
		{
			handledInOrder[0] = 2;
			handledInOrder[1] = 0;
			handledInOrder[2] = 1;
		}
	}
	else if(depth0 > depth2)
	{
		handledInOrder[0] = 1;
		handledInOrder[1] = 0;
		handledInOrder[2] = 2;
	}
	else
	{
		if (depth1 > depth2)
		{
			handledInOrder[0] = 1;
			handledInOrder[1] = 2;
		}
		else
		{
			handledInOrder[0] = 2;
			handledInOrder[1] = 1;
		}

		handledInOrder[2] = 0;
	}

	produceAxisArrow(inputRaw[handledInOrder[2]], outputStream);
	produceAxisArrow(inputRaw[handledInOrder[1]], outputStream);
	produceAxisArrow(inputRaw[handledInOrder[0]], outputStream);
}

// Pixel shader
float4 coordinateSystemPixelShader(PixelInputType input) : SV_TARGET
{
	return input._color;
}
