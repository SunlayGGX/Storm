#pragma once


namespace Storm
{
	struct SerializeRecordHeader
	{
	public:
		struct ParticleSystemLayout
		{
			uint32_t _particleSystemId;
			std::size_t _particlesCount;
			bool _isFluid;
		};

	public:
		float _recordFrameRate;
		std::vector<Storm::SerializeRecordHeader::ParticleSystemLayout> _particleSystemLayouts;
	};
}
