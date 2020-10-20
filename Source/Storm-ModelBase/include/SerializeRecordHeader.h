#pragma once


namespace Storm
{
	struct SerializeParticleSystemLayout;
	struct SerializeConstraintLayout;

	struct SerializeRecordHeader
	{
	public:
		~SerializeRecordHeader();

	public:
		float _recordFrameRate;
		uint64_t _frameCount;
		std::vector<Storm::SerializeParticleSystemLayout> _particleSystemLayouts;
		std::vector<Storm::SerializeConstraintLayout> _contraintLayouts;
	};
}
