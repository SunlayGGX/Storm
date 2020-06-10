#include "FluidParticleSystem.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "GeneralSimulationData.h"


bool Storm::FluidParticleSystem::isFluids() const noexcept
{
	return true;
}

void Storm::FluidParticleSystem::initializeIteration()
{
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const Storm::Vector3 &gravity = configMgr.getGeneralSimulationData()._gravity;

	std::for_each(std::execution::par_unseq, std::begin(_accelerations), std::end(_accelerations), [&gravity](Storm::Vector3 &accel)
	{
		accel = gravity;
	});
}
