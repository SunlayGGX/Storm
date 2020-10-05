#pragma once


namespace Storm
{
	struct SerializeParticleSystemLayout
	{
		uint32_t _particleSystemId;
		uint64_t _particlesCount;
		bool _isFluid;
	};

	struct SerializeRecordHeader
	{
	public:
		float _recordFrameRate;
		uint64_t _frameCount;
		std::vector<Storm::SerializeParticleSystemLayout> _particleSystemLayouts;
	};
}
