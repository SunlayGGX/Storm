cbuffer ConstantBuffer
{
	// ViewMatrix * ProjectionMatrix
	matrix _viewMatrix;
	matrix _projMatrix;

	float _particleRadius;

	float _nearPlaneDist;
};

struct VertexInputType
{
	float4 _position : POSITION;
	float4 _color : COLOR;
};

struct GeometryInputType
{
	float4 _position : POSITION;
	float4 _color : COLOR;
};

struct PixelInputType
{
	float4 _position : SV_POSITION;

	float4 _color : COLOR;

	float2 _miscData : TEXCOORD0;
};


// Vertex Shader
GeometryInputType particleVertexShader(VertexInputType input)
{
	GeometryInputType output;

	// Change the position vector to be 4 units for proper matrix calculations.
	//output._position = input._position;
	output._position = mul(input._position, _viewMatrix);

#if STORM_SELECTED_PARTICLE_ON_TOP
	if (input._color.x == 1.f && input._color.y == 1.f && input._color.z == 1.f)
	{
		output._position.z = _nearPlaneDist;
	}
#endif

	output._color = input._color;

	return output;
}

[maxvertexcount(6)]
void particleGeometryShader(point GeometryInputType inputRaw[1], inout TriangleStream<PixelInputType> outputStream)
{
	GeometryInputType input = inputRaw[0];

	PixelInputType corner1;
	corner1._position = float4(input._position.x + _particleRadius, input._position.y + _particleRadius, input._position.zw);
	corner1._position = mul(corner1._position, _projMatrix);
	corner1._color = input._color;
	corner1._miscData.x = 1.f;
	corner1._miscData.y = 1.f;

	PixelInputType corner2;
	corner2._position = float4(input._position.x - _particleRadius, input._position.y + _particleRadius, input._position.zw);
	corner2._position = mul(corner2._position, _projMatrix);
	corner2._color = input._color;
	corner2._miscData.x = -1.f;
	corner2._miscData.y = 1.f;

	PixelInputType corner3;
	corner3._position = float4(input._position.x + _particleRadius, input._position.y - _particleRadius, input._position.zw);
	corner3._position = mul(corner3._position, _projMatrix);
	corner3._color = input._color;
	corner3._miscData.x = 1.f;
	corner3._miscData.y = -1.f;

	PixelInputType corner4;
	corner4._position = float4(input._position.x - _particleRadius, input._position.y - _particleRadius, input._position.zw);
	corner4._position = mul(corner4._position, _projMatrix);
	corner4._color = input._color;
	corner4._miscData.x = -1.f;
	corner4._miscData.y = -1.f;

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
float4 particlePixelShader(PixelInputType input) : SV_TARGET
{
	const float uvLength = length(input._miscData.xy);
	if (uvLength > 1.f)
	{
		discard;
	}
	else if (uvLength > 0.95f)
	{
		input._color = float4(1.f, 1.f, 1.f, 1.f);
	}

	return input._color;
}
