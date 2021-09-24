#pragma once


#include "NonInstanciable.h"
#include "FluidParticleSystem.h"


namespace Storm
{
	class FluidParticleSystemUtils : private Storm::NonInstanciable
	{
	public:
		enum class NeighborType
		{
			Fluid,
			StaticRb,
			DynamicRb,
		};

	public:
		template<class Func>
		static void forEachNeighbor(const Storm::FluidParticleSystem &fluidPSystem, const std::size_t currentPIndex, const Func &func, const bool applyOnRbs = true)
		{
			const Storm::FluidParticleSystem::ParticleNeighborhoodPartitioner &currentPPartitioner = fluidPSystem.getNeighborhoodPartitioner()[currentPIndex];

			auto neighborIter = currentPPartitioner._fluidIter;

			// Fluids
			for (; neighborIter != currentPPartitioner._staticRbIter; ++neighborIter)
			{
				func.operator()<Storm::FluidParticleSystemUtils::NeighborType::Fluid>(*neighborIter);
			}

			// RigidBodies
			if (applyOnRbs)
			{
				// Statics
				for (; neighborIter != currentPPartitioner._dynamicRbIter; ++neighborIter)
				{
					func.operator()<Storm::FluidParticleSystemUtils::NeighborType::StaticRb>(*neighborIter);
				}

				// Dynamics
				for (; neighborIter != currentPPartitioner._endIter; ++neighborIter)
				{
					func.operator()<Storm::FluidParticleSystemUtils::NeighborType::DynamicRb>(*neighborIter);
				}
			}
		}


		template<class Func>
		static void forEachRigidbodyNeighbor(const Storm::FluidParticleSystem &fluidPSystem, const std::size_t currentPIndex, const Func &func)
		{
			const Storm::FluidParticleSystem::ParticleNeighborhoodPartitioner &currentPPartitioner = fluidPSystem.getNeighborhoodPartitioner()[currentPIndex];

			auto neighborIter = currentPPartitioner._staticRbIter;

			// Statics
			for (; neighborIter < currentPPartitioner._dynamicRbIter; ++neighborIter)
			{
				func.operator()<Storm::FluidParticleSystemUtils::NeighborType::StaticRb>(*neighborIter);
			}

			// Dynamics
			for (; neighborIter < currentPPartitioner._endIter; ++neighborIter)
			{
				func.operator()<Storm::FluidParticleSystemUtils::NeighborType::DynamicRb>(*neighborIter);
			}
		}
	};
}
