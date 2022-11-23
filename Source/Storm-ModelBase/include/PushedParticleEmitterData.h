#pragma once


namespace Storm
{
	struct PushedParticleEmitterData
	{
	public:
		struct ProcessedData
		{
			Storm::Vector3 _position;
			float _alphaCoeff;
		};

	public:
		unsigned int _id;
		std::vector<ProcessedData> _data;
	};
}
