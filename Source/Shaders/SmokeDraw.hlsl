cbuffer ConstantBuffer
{
	// ViewMatrix * ProjectionMatrix
	matrix _viewMatrix;
	matrix _projMatrix;

	float4 _generalColor;
	
	float _dimension;
	float _persistentReduce;

	float _padding;
};

Texture2D perlinTexture : register(t0);
Texture2D frameBeforeTexture : register(t1);

SamplerState perlinTextureSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct VertexInputType
{
	float3 _position : POSITION;
	float _alphaCoeff : BLENDWEIGHT0;
};

struct GeometryInputType
{
	float4 _position : POSITION;
	float _alphaCoeff : TEXCOORD0;
};

struct PixelInputType
{
	float4 _position : SV_POSITION;

	float2 _uv : TEXCOORD0;
	float2 _truePos : TEXCOORD1;
	float _alphaCoeff : TEXCOORD2;
};


// Vertex Shader
GeometryInputType smokeVertexShader(VertexInputType input)
{
	GeometryInputType output;

	// Change the position vector to be 4 units for proper matrix calculations.
	//output._position = input._position;
	output._position = mul(float4(input._position.xyz, 1.f), _viewMatrix);
	output._alphaCoeff = input._alphaCoeff;

	return output;
}

[maxvertexcount(6)]
void smokeGeometryShader(point GeometryInputType inputRaw[1], inout TriangleStream<PixelInputType> outputStream)
{
	GeometryInputType input = inputRaw[0];

	PixelInputType corner1;
	corner1._position = float4(input._position.x + _dimension, input._position.y + _dimension, input._position.zw);
	corner1._position = mul(corner1._position, _projMatrix);
	corner1._uv.x = 1.f;
	corner1._uv.y = 1.f;
	corner1._truePos = input._position;
	corner1._alphaCoeff = input._alphaCoeff;

	PixelInputType corner2;
	corner2._position = float4(input._position.x - _dimension, input._position.y + _dimension, input._position.zw);
	corner2._position = mul(corner2._position, _projMatrix);
	corner2._uv.x = 0.f;
	corner2._uv.y = 1.f;
	corner2._truePos = input._position;
	corner2._alphaCoeff = input._alphaCoeff;

	PixelInputType corner3;
	corner3._position = float4(input._position.x + _dimension, input._position.y - _dimension, input._position.zw);
	corner3._position = mul(corner3._position, _projMatrix);
	corner3._uv.x = 1.f;
	corner3._uv.y = 0.f;
	corner3._truePos = input._position;
	corner3._alphaCoeff = input._alphaCoeff;

	PixelInputType corner4;
	corner4._position = float4(input._position.x - _dimension, input._position.y - _dimension, input._position.zw);
	corner4._position = mul(corner4._position, _projMatrix);
	corner4._uv.x = 0.f;
	corner4._uv.y = 0.f;
	corner4._truePos = input._position;
	corner4._alphaCoeff = input._alphaCoeff;

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
float4 smokePixelShader(PixelInputType input) : SV_TARGET
{
	float alphaEdge = (1.f - abs(input._uv.x - 0.5f) * 2.f) * (1.f - abs(input._uv.y - 0.5f) * 2.f);

	float4 persistentSmoke = frameBeforeTexture.Sample(perlinTextureSampler, input._uv) * _persistentReduce;

	/*float4 threshold = step(persistentSmoke, float4(0.05f, 0.05f, 0.05f, 0.05f));
	persistentSmoke = persistentSmoke * (threshold.r * threshold.g * threshold.b * threshold.a);*/

	if ((persistentSmoke.r * persistentSmoke.g * persistentSmoke.b * persistentSmoke.a) < 0.00005f)
	{
		persistentSmoke = float4(0.f, 0.f, 0.f, 0.f);
	}

	float4 colorCoeff = float4(_generalColor.rgb, _generalColor.a * input._alphaCoeff * alphaEdge);
	return perlinTexture.Sample(perlinTextureSampler, input._uv) * colorCoeff + persistentSmoke;
}
