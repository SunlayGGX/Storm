cbuffer ConstantBuffer
{
    // ViewMatrix * ProjectionMatrix
    matrix _viewMatrix;
    matrix _projMatrix;
};

struct VertexInputType
{
    float4 _position : POSITION;
};
