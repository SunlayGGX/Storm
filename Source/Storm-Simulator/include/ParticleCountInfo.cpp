#include "ParticleCountInfo.h"

#include "ParticleSystem.h"
#include "RigidBodyParticleSystem.h"

#include "ThreadingSafety.h"

#include "MemoryHelper.h"


Storm::ParticleCountInfo::ParticleCountInfo(const Storm::ParticleSystemContainer &pSystemContainer)
{
	assert(Storm::isSimulationThread() && "This method should only be used inside the simulation thread!");

	Storm::ZeroMemories(*this);

	for (const auto &pSystemPair : pSystemContainer)
	{
		const Storm::ParticleSystem &pSystem = *pSystemPair.second;

		const std::size_t currentPSystemPCount = pSystem.getParticleCount();

		_totalParticleCount += currentPSystemPCount;
		if (pSystem.isFluids())
		{
			_fluidParticleCount += currentPSystemPCount;
		}
		else
		{
			_rigidbodiesParticleCount += currentPSystemPCount;
			if (static_cast<const Storm::RigidBodyParticleSystem &>(pSystem).isStatic())
			{
				_staticRigidbodiesParticleCount += currentPSystemPCount;
			}
			else
			{
				_dynamicRigidbodiesParticleCount += currentPSystemPCount;
			}
		}
	}
}
