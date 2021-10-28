cbuffer ConstantBuffer
{
	matrix viewProjMatrix;
};

Texture2D g_mappedDepth[2];
Texture2D g_textureToMerge[2];
SamplerState samplerSt;

struct VertexInputType
{
	float4 _position : POSITION;
};

struct PixelInputType
{
	float4 _position : SV_POSITION;
	float2 _uv : TEXCOORD0;
};

// Vertex Shader
PixelInputType textureMergerDepthVertexShader(VertexInputType input)
{
	PixelInputType output;

	output._position = mul(input._position, viewProjMatrix);
	output._uv = output._position.xy;

	return output;
}

// Pixel shader
float4 textureMergerDepthPixelShader(PixelInputType input) : SV_TARGET
{
	float depthText0 = g_mappedDepth[0].Sample(samplerSt, input._uv);
	float depthText1 = g_mappedDepth[1].Sample(samplerSt, input._uv);

	if (depthText0 < depthText1)
	{
		return g_textureToMerge[0].Sample(samplerSt, input._uv);
	}
	else
	{
		return g_textureToMerge[1].Sample(samplerSt, input._uv);
	}
}
