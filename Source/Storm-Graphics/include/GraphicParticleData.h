#pragma once


namespace Storm
{
	struct GraphicParticleData
	{
	public:
		using PointType = DirectX::XMVECTOR;
		using ColorType = DirectX::XMVECTOR;

	public:
		GraphicParticleData(const Storm::Vector3 &position, const ColorType &color);

	public:
		PointType _pos;
		ColorType _color;
	};
}
