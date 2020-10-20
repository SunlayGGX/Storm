#pragma once

#include "NonInstanciable.h"
#include "ParticleSystemContainer.h"


namespace Storm
{
	class ParticleSystem;
	struct SerializeRecordPendingData;
	struct SerializeRecordContraintsData;

	class ReplaySolver : private Storm::NonInstanciable
	{
	public:
		static void transferFrameToParticleSystem_move(Storm::ParticleSystemContainer &particleSystems, Storm::SerializeRecordPendingData &frameFrom);
		static void transferFrameToParticleSystem_copy(Storm::ParticleSystemContainer &particleSystems, const Storm::SerializeRecordPendingData &frameFrom);

		static void computeNextRecordTime(float &inOutNextRecordTime, const float currentPhysicsTime, const float recordFps);

		static bool replayCurrentNextFrame(Storm::ParticleSystemContainer &particleSystems, Storm::SerializeRecordPendingData &frameBefore, Storm::SerializeRecordPendingData &frameAfter, const float recordFps, std::vector<Storm::SerializeRecordContraintsData> &outFrameConstraintData);
	};
}
