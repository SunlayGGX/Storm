cbuffer ConstantBuffer
{
	// ViewMatrix * ProjectionMatrix
	matrix _viewMatrix;
	matrix _projMatrix;

	float4 _generalColor;
	
	float2 _textureDimension;
	
	float _nearPlaneDist;
	
	float _padding;
};

Texture2D perlinTexture : register(t0);

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

	float2 _miscData : TEXCOORD0;
	float _clipDist : SV_ClipDistance0;
};


// Vertex Shader
GeometryInputType smokeVertexShader(VertexInputType input)
{
	GeometryInputType output;

	// Change the position vector to be 4 units for proper matrix calculations.
	//output._position = input._position;
	output._position = mul(float4(input._position.xyz, 1.f), _viewMatrix);

	return output;
}

[maxvertexcount(6)]
void smokeGeometryShader(point GeometryInputType inputRaw[1], inout TriangleStream<PixelInputType> outputStream)
{
	GeometryInputType input = inputRaw[0];

	// A plane is given by a vector and a point. xyz are the vector coordinate (here in view space coordinate since I'll be applying it on the position multiplied by matView).
	// w is the offset from the origin given by the xyz vector (plane vector). Therefore, since I'm in view coordinate, this is camPos + w * xyz.
	// => Since in view space coordinate, the look vector is { 0, 0, 1 }, we have :
	// Near plane is therefore given by the point P = camPos + _nearPlaneDist * lookVect, and the plane normal is given by lookVect.
	// The minus is because w is inverted.
	const float4 nearPlane = float4(0.0, 0.0, 1.0, -_nearPlaneDist);

	// Everyone has the same since in view space coordinate, the vertex would become a plane that faces the camera. Therefore, we could assume that if one vertex of the plane is before the clipping plane then all vertexes are, and if one is after then everyone are after.
	const float clipDistEveryone = dot(input._position, nearPlane);

	PixelInputType corner1;
	corner1._position = float4(input._position.x + _textureDimension.x, input._position.y + _textureDimension.y, input._position.zw);
	corner1._position = mul(corner1._position, _projMatrix);
	corner1._miscData.x = 1.f;
	corner1._miscData.y = 1.f;
	corner1._clipDist = clipDistEveryone;

	PixelInputType corner2;
	corner2._position = float4(input._position.x - _textureDimension.x, input._position.y + _textureDimension.y, input._position.zw);
	corner2._position = mul(corner2._position, _projMatrix);
	corner2._miscData.x = -1.f;
	corner2._miscData.y = 1.f;
	corner2._clipDist = clipDistEveryone;

	PixelInputType corner3;
	corner3._position = float4(input._position.x + _textureDimension.x, input._position.y - _textureDimension.y, input._position.zw);
	corner3._position = mul(corner3._position, _projMatrix);
	corner3._miscData.x = 1.f;
	corner3._miscData.y = -1.f;
	corner3._clipDist = clipDistEveryone;

	PixelInputType corner4;
	corner4._position = float4(input._position.x - _textureDimension.x, input._position.y - _textureDimension.y, input._position.zw);
	corner4._position = mul(corner4._position, _projMatrix);
	corner4._miscData.x = -1.f;
	corner4._miscData.y = -1.f;
	corner4._clipDist = clipDistEveryone;

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
	//const float uvLength = length(input._miscData.xy);
	
	// TODO : Blend
	
	return _generalColor;
}
