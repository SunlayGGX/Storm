#include "Cage.h"

#include "ITimeManager.h"
#include "IRandomManager.h"
#include "SingletonHolder.h"
#include "SimulatorManager.h"

#include "SceneCageConfig.h"

#include "ParticleSystem.h"

#include "RunnerHelper.h"
#include "Vector3Utils.h"


namespace
{
	template<class ValueType, class SelectorFunc>
	void reflectWithPenalty(const ValueType &min, const ValueType &max, const ValueType &diff, const ValueType &penaltyLowest, const ValueType &penaltyHighest, ValueType &valuePos, ValueType &valueVel, SelectorFunc &&selector)
	{
		auto &selectedPosCoord = selector(valuePos);
		auto &selectedVelCoord = selector(valueVel);
		const float minValue = selector(min);
		const float maxValue = selector(max);
		const float diffValue = selector(diff);

		if (selectedPosCoord < minValue)
		{
			const float underflow = selectedPosCoord - minValue;

			const float remainder = std::remainder(underflow, diffValue);
			selectedPosCoord = maxValue + remainder * diffValue;
			
			const float selectedPenalty = selector(penaltyLowest);
			selectedVelCoord *= selectedPenalty;
		}
		else if (selectedPosCoord > maxValue)
		{
			const float overflow = selectedPosCoord - maxValue;

			const float remainder = std::remainder(overflow, diffValue);
			selectedPosCoord = minValue + remainder * diffValue;

			const float selectedPenalty = selector(penaltyHighest);
			selectedVelCoord *= selectedPenalty;
		}
	}
}


Storm::Cage::Cage(const Storm::SceneCageConfig &sceneCageConfig) :
	_boxMin{ sceneCageConfig._boxMin },
	_boxMax{ sceneCageConfig._boxMax },
	_killY{ sceneCageConfig._rbSimulKillY },
	_infiniteDomain{ sceneCageConfig._infiniteDomain },
	_velocityCoeffsLeftBottomFront{ sceneCageConfig._passthroughVelReduceCoeffLeftBottomFront },
	_passthroughVelReduceCoeffRightTopBack{ sceneCageConfig._passthroughVelReduceCoeffRightTopBack }
{

}

Storm::Cage::~Cage() = default;

void Storm::Cage::doEnclose(Storm::ParticleSystemContainer &pSystems) const
{
	constexpr auto xSelector = [](auto &vect) -> auto& { return vect.x(); };
	constexpr auto ySelector = [](auto &vect) -> auto& { return vect.y(); };
	constexpr auto zSelector = [](auto &vect) -> auto& { return vect.z(); };

	if (_infiniteDomain)
	{
		const Storm::Vector3 diff = _boxMax - _boxMin;
		for (const auto &pSystemPair : pSystems)
		{
			Storm::ParticleSystem &pSystem = *pSystemPair.second;
			if (pSystem.isFluids())
			{
				std::vector<Storm::Vector3> &allPPositions = pSystem.getPositions();
				std::vector<Storm::Vector3> &allPVelocities = pSystem.getVelocity();

				const float maxDisplacement = Storm::SimulatorManager::instance().getKernelLength();

				Storm::runParallel(allPPositions, [this, &diff, &allPVelocities, &xSelector, &ySelector, &zSelector](Storm::Vector3 &currentPPosition, const std::size_t currentPIndex)
				{
					reflectWithPenalty(_boxMin, _boxMax, diff, _velocityCoeffsLeftBottomFront, _passthroughVelReduceCoeffRightTopBack, currentPPosition, allPVelocities[currentPIndex], xSelector);
					reflectWithPenalty(_boxMin, _boxMax, diff, _velocityCoeffsLeftBottomFront, _passthroughVelReduceCoeffRightTopBack, currentPPosition, allPVelocities[currentPIndex], ySelector);
					reflectWithPenalty(_boxMin, _boxMax, diff, _velocityCoeffsLeftBottomFront, _passthroughVelReduceCoeffRightTopBack, currentPPosition, allPVelocities[currentPIndex], zSelector);
				});
			}
#if false
			else
			{
				// TODO : I need to think for this case...
				// because rigid bodies are solid, their particles shouldn't move from each other (this is a constraint) therefore they cannot be teared apart from one another...
				// This constraint is true for my engine, but also for the Physics engine I use.

				// Finally, the physics engine handle the case by preventing the rigid body to leave the cage.
			}
#endif
		}
	}
	else
	{
		const Storm::Vector3 cageCenter = (_boxMax + _boxMin) / 2.f;
		Storm::IRandomManager &randomMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IRandomManager>();

		for (const auto &pSystemPair : pSystems)
		{
			Storm::ParticleSystem &pSystem = *pSystemPair.second;
			if (pSystem.isFluids())
			{
				std::vector<Storm::Vector3> &allPPositions = pSystem.getPositions();
				std::vector<Storm::Vector3> &allPVelocities = pSystem.getVelocity();

				const float maxDisplacement = Storm::SimulatorManager::instance().getKernelLength();

				Storm::runParallel(allPPositions,
					[
						this,
						&cageCenter,
						&randomMgr,
						&allPVelocities,
						maxDisplacement,
						&xSelector,
						&ySelector,
						&zSelector
					](Storm::Vector3 &currentPPosition, const std::size_t currentPIndex)
				{
					bool res = Storm::clampInPlace(_boxMin, _boxMax, currentPPosition, xSelector);
					res |= Storm::clampInPlace(_boxMin, _boxMax, currentPPosition, ySelector);
					res |= Storm::clampInPlace(_boxMin, _boxMax, currentPPosition, zSelector);

					if (res)
					{
						// Move randomly the particle to the center with a maximum of maxDisplacement.
						const float randomDisplacement = randomMgr.randomizeFloat() * maxDisplacement;
						currentPPosition += (cageCenter - currentPPosition) * randomDisplacement;

						allPVelocities[currentPIndex].setZero();
					}
				});
			}
		}
	}

	if (!std::isnan(_killY))
	{
		for (const auto &pSystemPair : pSystems)
		{
			const Storm::ParticleSystem &pSystem = *pSystemPair.second;
			if (!pSystem.isFluids() && !pSystem.isStatic())
			{
				const std::vector<Storm::Vector3> &positions = pSystem.getPositions();
				if (std::any_of(std::execution::par, std::begin(positions), std::end(positions), [killY = _killY](const Storm::Vector3 &currentPPosition)
				{
					return currentPPosition.y() < killY;
				}))
				{
					LOG_COMMENT << "At least one particle went below " << _killY << " y and killY was enabled, therefore as expected, the simulation will stop because we consider this is useless to pursue it further.";
					Storm::SingletonHolder::instance().getSingleton<Storm::ITimeManager>().quit();
				}
			}
		}
	}
}
