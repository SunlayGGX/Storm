#include "Cage.h"

#include "IRandomManager.h"
#include "SingletonHolder.h"
#include "SimulatorManager.h"

#include "SceneCageConfig.h"

#include "ParticleSystem.h"

#include "RunnerHelper.h"
#include "Vector3Utils.h"


Storm::Cage::Cage(const Storm::SceneCageConfig &sceneCageConfig) :
	_boxMin{ sceneCageConfig._boxMin },
	_boxMax{ sceneCageConfig._boxMax }
{

}

Storm::Cage::~Cage() = default;

void Storm::Cage::doEnclose(Storm::ParticleSystemContainer &pSystems) const
{
	const auto xSelector = [](auto &vect) -> auto& { return vect.x(); };
	const auto ySelector = [](auto &vect) -> auto& { return vect.y(); };
	const auto zSelector = [](auto &vect) -> auto& { return vect.z(); };

	const Storm::Vector3 cageCenter = (_boxMax + _boxMin) / 2.f;
	Storm::IRandomManager &randomMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IRandomManager>();

	for (const auto &pSystemPair : pSystems)
	{
		Storm::ParticleSystem &pSystem = *pSystemPair.second;
		if (pSystem.isFluids())
		{
			std::vector<Storm::Vector3> &allPPositions = pSystem.getPositions();
			std::vector<Storm::Vector3> &allPVelocities = pSystem.getVelocity();

			const float maxDisplacment = Storm::SimulatorManager::instance().getKernelLength();

			Storm::runParallel(allPPositions,
				[
				this,
				&cageCenter,
				&randomMgr,
				&allPVelocities,
				maxDisplacment,
				xSelector = [](auto &vect) -> auto& { return vect.x(); },
				ySelector = [](auto &vect) -> auto& { return vect.y(); },
				zSelector = [](auto &vect) -> auto& { return vect.z(); }
				](Storm::Vector3 &currentPPosition, const std::size_t currentPIndex)
			{
				bool res = Storm::clampInPlace(_boxMin, _boxMax, currentPPosition, xSelector);
				res |= Storm::clampInPlace(_boxMin, _boxMax, currentPPosition, ySelector);
				res |= Storm::clampInPlace(_boxMin, _boxMax, currentPPosition, zSelector);

				if (res)
				{
					// Move randomly the particle to the center with a maximum of maxDisplacment.
					const float randomDisplacment = randomMgr.randomizeFloat() * maxDisplacment;
					currentPPosition += (cageCenter - currentPPosition) * randomDisplacment;

					allPVelocities[currentPIndex].setZero();
				}
			});
		}
	}
}
