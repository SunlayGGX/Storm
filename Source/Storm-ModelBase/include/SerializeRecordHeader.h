#pragma once


namespace Storm
{
	struct SerializeParticleSystemLayout;
	struct SerializeConstraintLayout;
	struct SerializeSupportedFeatureLayout;

	struct SerializeRecordHeader
	{
	public:
		SerializeRecordHeader();
		SerializeRecordHeader(SerializeRecordHeader &&);
		~SerializeRecordHeader();

		SerializeRecordHeader& operator=(SerializeRecordHeader &&);

	public:
		float _recordFrameRate;
		uint8_t _infiniteDomain;
		uint64_t _frameCount;
		std::vector<Storm::SerializeParticleSystemLayout> _particleSystemLayouts;
		std::vector<Storm::SerializeConstraintLayout> _contraintLayouts;
		std::shared_ptr<Storm::SerializeSupportedFeatureLayout> _supportedFeaturesLayout;
	};
}
