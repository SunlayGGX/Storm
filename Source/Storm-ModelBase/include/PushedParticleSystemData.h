#pragma once


namespace Storm
{
	struct PushedParticleSystemDataParameter
	{
	public:
		unsigned int _particleSystemId;
		
		const std::vector<Storm::Vector3>* _positionsData;
		
		const std::vector<Storm::Vector3>* _velocityData;
		const std::vector<float>* _densityData;
		const std::vector<float>* _pressureData;

		bool _isFluids;
		bool _isWall;
	};
}
