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
		float _kernelLength;
		std::vector<Storm::SerializeRecordParticleSystemData> _particleSystemElements;
		std::vector<Storm::SerializeRecordContraintsData> _constraintElements;
	};
}
