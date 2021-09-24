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

			const auto* neighborPtr = currentPNeighborhoods.data();

			// Fluids
			{
				const auto*const rbNeighborPtr = currentPNeighborhoods.data() + currentPPartitioner._staticRbIndex;
				for (; neighborPtr < rbNeighborPtr; ++neighborPtr)
				{
					func.operator()<Storm::FluidParticleSystemUtils::NeighborType::Fluid>(*neighborPtr);
				}
			}

			// RigidBodies
			if (applyOnRbs)
			{
				// Statics
				{
					const auto*const dynamicNeighborPtr = currentPNeighborhoods.data() + currentPPartitioner._dynamicRbIndex;
					for (; neighborPtr < dynamicNeighborPtr; ++neighborPtr)
					{
						func.operator()<Storm::FluidParticleSystemUtils::NeighborType::StaticRb>(*neighborPtr);
					}
				}

				// Dynamics
				const auto*const endNeighborhoodsPtr = currentPNeighborhoods.data() + currentPNeighborhoods.size();
				for (; neighborPtr < endNeighborhoodsPtr; ++neighborPtr)
				{
					func.operator()<Storm::FluidParticleSystemUtils::NeighborType::DynamicRb>(*neighborPtr);
				}
			}
		}


		template<class Func>
		static void forEachRigidbodyNeighbor(const Storm::FluidParticleSystem &fluidPSystem, const std::size_t currentPIndex, const Func &func)
		{
			const Storm::ParticleNeighborhoodArray &currentPNeighborhoods = fluidPSystem.getNeighborhoodArrays()[currentPIndex];
			const Storm::FluidParticleSystem::ParticleNeighborhoodPartitioner &currentPPartitioner = fluidPSystem.getNeighborhoodPartitioner()[currentPIndex];

			const auto* neighborPtr = currentPNeighborhoods.data();

			// Statics
			{
				const auto*const dynamicNeighborPtr = currentPNeighborhoods.data() + currentPPartitioner._dynamicRbIndex;
				for (; neighborPtr < dynamicNeighborPtr; ++neighborPtr)
				{
					func.operator()<Storm::FluidParticleSystemUtils::NeighborType::StaticRb>(*neighborPtr);
				}
			}

			// Dynamics
			const auto*const endNeighborhoodsPtr = currentPNeighborhoods.data() + currentPNeighborhoods.size();
			for (; neighborPtr < endNeighborhoodsPtr; ++neighborPtr)
			{
				func.operator()<Storm::FluidParticleSystemUtils::NeighborType::DynamicRb>(*neighborPtr);
			}
		}
	};
}
