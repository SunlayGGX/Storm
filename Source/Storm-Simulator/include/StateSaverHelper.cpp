#include "StateSaverHelper.h"

#include "SingletonHolder.h"
#include "ITimeManager.h"

#include "ParticleSystem.h"
#include "FluidParticleSystem.h"
#include "RigidBodyParticleSystem.h"

#include "SimulationState.h"
#include "SystemSimulationStateObject.h"

#include "RunnerHelper.h"

#define STORM_HIJACKED_TYPE Storm::Vector3
#	include "VectHijack.h"
#undef STORM_HIJACKED_TYPE

#define STORM_HIJACKED_TYPE float
#	include "VectHijack.h"
#undef STORM_HIJACKED_TYPE


namespace
{
	template<class Type>
	void reserveResizeUnitialized(std::vector<Type> &outVect, const Storm::VectorHijacker &hijacker)
	{
		outVect.reserve(hijacker._newSize);
		Storm::setNumUninitialized_hijack(outVect, hijacker);
	}
}


void Storm::StateSaverHelper::saveIntoState(Storm::SimulationState &state, const Storm::ParticleSystemContainer &pSystemContainer)
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();

	state._currentPhysicsTime = timeMgr.getCurrentPhysicsElapsedTime();

	state._pSystemStates.reserve(pSystemContainer.size());
	for (const auto &pSystemPair : pSystemContainer)
	{
		const Storm::ParticleSystem &pSystem = static_cast<const Storm::ParticleSystem &>(*pSystemPair.second);

		const std::vector<Storm::Vector3> &positions = pSystem.getPositions();
		const std::vector<Storm::Vector3> &velocities = pSystem.getVelocity();
		const std::vector<Storm::Vector3> &forces = pSystem.getForces();

		Storm::VectorHijacker hijacker{ positions.size() };

		Storm::SystemSimulationStateObject &pSystemState = state._pSystemStates.emplace_back();

		reserveResizeUnitialized(pSystemState._positions, hijacker);
		reserveResizeUnitialized(pSystemState._velocities, hijacker);
		reserveResizeUnitialized(pSystemState._forces, hijacker);

		if (pSystem.isFluids())
		{
			const Storm::FluidParticleSystem &pSystemAsFluid = static_cast<const Storm::FluidParticleSystem &>(pSystem);

			pSystemState._isFluid = true;
			pSystemState._isStatic = false;

			const std::vector<float> &densities = pSystemAsFluid.getDensities();
			const std::vector<float> &pressures = pSystemAsFluid.getPressures();
			const std::vector<float> &masses = pSystemAsFluid.getMasses();

			reserveResizeUnitialized(pSystemState._densities, hijacker);
			reserveResizeUnitialized(pSystemState._pressures, hijacker);
			reserveResizeUnitialized(pSystemState._masses, hijacker);

			Storm::runParallel(positions, [&](const Storm::Vector3 &currentPPosition, const std::size_t currentPIndex)
			{
				pSystemState._positions[currentPIndex] = currentPPosition;
				pSystemState._velocities[currentPIndex] = velocities[currentPIndex];
				pSystemState._forces[currentPIndex] = forces[currentPIndex];
				pSystemState._densities[currentPIndex] = densities[currentPIndex];
				pSystemState._pressures[currentPIndex] = pressures[currentPIndex];
				pSystemState._masses[currentPIndex] = masses[currentPIndex];
			});
		}
		else
		{
			const Storm::RigidBodyParticleSystem &pSystemAsRb = static_cast<const Storm::RigidBodyParticleSystem &>(pSystem);

			pSystemState._isFluid = false;
			pSystemState._isStatic = pSystemAsRb.isStatic();

			const std::vector<float> &volumes = pSystemAsRb.getVolumes();

			pSystemState._globalPosition = pSystemAsRb.getRbPosition();
			reserveResizeUnitialized(pSystemState._volumes, hijacker);

			Storm::runParallel(positions, [&](const Storm::Vector3 &currentPPosition, const std::size_t currentPIndex)
			{
				pSystemState._positions[currentPIndex] = currentPPosition;
				pSystemState._velocities[currentPIndex] = velocities[currentPIndex];
				pSystemState._forces[currentPIndex] = forces[currentPIndex];
				pSystemState._volumes[currentPIndex] = volumes[currentPIndex];
			});
		}
	}
}
