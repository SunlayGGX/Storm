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
			const Storm::ParticleNeighborhoodArray &currentPNeighborhoods = fluidPSystem.getNeighborhoodArrays()[currentPIndex];
			const Storm::FluidParticleSystem::ParticleNeighborhoodPartitioner &currentPPartitioner = fluidPSystem.getNeighborhoodPartitioner()[currentPIndex];

			std::size_t neighborhoodIter = 0;

			// Fluids
			for (; neighborhoodIter < currentPPartitioner._staticRbIndex; ++neighborhoodIter)
			{
				func.operator()<Storm::FluidParticleSystemUtils::NeighborType::Fluid>(currentPNeighborhoods[neighborhoodIter]);
			}

			// RigidBodies
			if (applyOnRbs)
			{
				// Statics
				for (; neighborhoodIter < currentPPartitioner._dynamicRbIndex; ++neighborhoodIter)
				{
					func.operator()<Storm::FluidParticleSystemUtils::NeighborType::StaticRb>(currentPNeighborhoods[neighborhoodIter]);
				}

				// Dynamics
				const std::size_t endNeighborhoods = currentPNeighborhoods.size();
				for (; neighborhoodIter < endNeighborhoods; ++neighborhoodIter)
				{
					func.operator()<Storm::FluidParticleSystemUtils::NeighborType::DynamicRb>(currentPNeighborhoods[neighborhoodIter]);
				}
			}
		}


		template<class Func>
		static void forEachRigidbodyNeighbor(const Storm::FluidParticleSystem &fluidPSystem, const std::size_t currentPIndex, const Func &func)
		{
			const Storm::ParticleNeighborhoodArray &currentPNeighborhoods = fluidPSystem.getNeighborhoodArrays()[currentPIndex];
			const Storm::FluidParticleSystem::ParticleNeighborhoodPartitioner &currentPPartitioner = fluidPSystem.getNeighborhoodPartitioner()[currentPIndex];

			std::size_t neighborhoodIter = currentPPartitioner._staticRbIndex;

			// Statics
			for (; neighborhoodIter < currentPPartitioner._dynamicRbIndex; ++neighborhoodIter)
			{
				func.operator()<Storm::FluidParticleSystemUtils::NeighborType::StaticRb>(currentPNeighborhoods[neighborhoodIter]);
			}

			// Dynamics
			const std::size_t endNeighborhoods = currentPNeighborhoods.size();
			for (; neighborhoodIter < endNeighborhoods; ++neighborhoodIter)
			{
				func.operator()<Storm::FluidParticleSystemUtils::NeighborType::DynamicRb>(currentPNeighborhoods[neighborhoodIter]);
			}
		}
	};
}
