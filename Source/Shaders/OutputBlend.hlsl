cbuffer ConstantBuffer
{
	float _persistentReduce;

	float3 _padding;
};

Texture2D toBlend : register(t0);

SamplerState textureSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct VertexInputType
{
	float4 _position : POSITION;
	float2 _uv : TEXCOORD0;
};

struct PixelInputType
{
	float4 _position : SV_POSITION;
	float2 _uv : TEXCOORD0;
};

PixelInputType blendVertexShader(uint vertexID : SV_VertexID)
{
	PixelInputType output;

	output._uv = float2((vertexID << 1) & 2, vertexID & 2);
	output._position = float4(output._uv * float2(2.f, -2.f) + float2(-1.f, 1.f), 0.f, 1.f);

	return output;
}

// Pixel shader
float4 blendPixelShader(PixelInputType input) : SV_TARGET
{
	// We'll let the output merger blend with the render target.
	float4 outColor = toBlend.Sample(textureSampler, input._uv) * _persistentReduce;

#if true
	float4 threshold = step(outColor, float4(0.005f, 0.005f, 0.005f, 0.005f));
	outColor *= (threshold.x * threshold.y * threshold.z * threshold.w);
#else
	if ((outColor.r * outColor.g * outColor.b * outColor.a) < 0.0000005f)
	{
		outColor = float4(0.f, 0.f, 0.f, 0.f);
	}
#endif

	return outColor;
}
