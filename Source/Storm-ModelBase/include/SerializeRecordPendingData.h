#pragma once


namespace Storm
{
	struct SerializeRecordParticleSystemData;
	struct SerializeRecordContraintsData;

	struct SerializeRecordPendingData
	{
	public:
		~SerializeRecordPendingData();

	public:
		float _physicsTime;
		std::vector<Storm::SerializeRecordParticleSystemData> _particleSystemElements;
		std::vector<Storm::SerializeRecordContraintsData> _constraintElements;
	};
}
