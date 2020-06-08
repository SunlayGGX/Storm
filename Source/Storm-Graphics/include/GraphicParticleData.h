#pragma once


namespace Storm
{
	struct GraphicParticleData
	{
	public:
		using PointType = DirectX::XMVECTOR;

	public:
		GraphicParticleData(const Storm::Vector3 &position, const float particleSize);

	public:
		PointType _pos;
		float _pointSize;
	};
}
