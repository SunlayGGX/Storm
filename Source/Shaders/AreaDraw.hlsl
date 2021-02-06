cbuffer ConstantBuffer
{
	matrix _worldMatrix;
	matrix _viewProjMatrix;
	float4 _eyePosition;

	float4 _areaColor;
};

struct VertexInputType
{
	float4 _position : POSITION;
	float4 _normal : NORMAL;
};

struct PixelInputType
{
	float4 _position : SV_POSITION;
	float _scalar : Output;
};

PixelInputType areaVertexShader(VertexInputType input)
{
	PixelInputType output;

	// We suppose we receive a vertex, not a vector. Therefore input._position.w should be 1.f.
	// input._position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output._position = mul(input._position, _worldMatrix);

#if defined(STORM_HIGHLIGHT_BORDER) && STORM_HIGHLIGHT_BORDER == true
	float4 dir = normalize(output._position - _eyePosition);
	dir.w = 0.f;

	output._scalar = abs(dot(input._normal, dir));
#endif

	output._position = mul(output._position, _viewProjMatrix);

	return output;
}

float4 areaPixelShader(PixelInputType input) : SV_TARGET
{
#if defined(STORM_HIGHLIGHT_BORDER) && STORM_HIGHLIGHT_BORDER == true
	float mask = input._scalar > 0.3f;
	return float4(_areaColor.xyz * mask, _areaColor.w);
#else
	return _areaColor;
#endif
}