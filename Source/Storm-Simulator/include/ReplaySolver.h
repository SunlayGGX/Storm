#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	class ParticleSystem;
	struct SerializeRecordPendingData;
	struct RecordConfigData;

	class ReplaySolver : private Storm::NonInstanciable
	{
	public:
		static void computeNextRecordTime(float &inOutNextRecordTime, const float currentPhysicsTime, const Storm::RecordConfigData &recordConfig);

		static bool replayCurrentNextFrame(std::map<unsigned int, std::unique_ptr<Storm::ParticleSystem>> &particleSystems, Storm::SerializeRecordPendingData &frameBefore, Storm::SerializeRecordPendingData &frameAfter, const Storm::RecordConfigData &recordConfig);
	};
}
