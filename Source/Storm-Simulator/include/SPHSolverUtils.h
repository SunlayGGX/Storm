#pragma once

#include "NonInstanciable.h"
#include "FluidParticleSystemUtils.h"


namespace Storm
{
	class SPHSolverUtils : private Storm::NonInstanciable
	{
	public:
		template<class DataContainerType>
		static void removeRawEndData(const unsigned int pSystemId, std::size_t toRemoveCount, DataContainerType &data)
		{
			if (auto found = data.find(pSystemId); found != std::end(data))
			{
				auto &data = found->second;
				while (toRemoveCount != 0)
				{
					data.pop_back();
					--toRemoveCount;
				}
			}
			else
			{
				Storm::throwException<Storm::Exception>("Cannot find particle system data bound to particle system " + std::to_string(pSystemId));
			}
		}

		__forceinline static void computeDragForce(const Storm::Vector3 &vi, const Storm::Vector3 &vj, const float dragPreCoeff, const float rij, Storm::Vector3 &outForce)
		{
			outForce = vj - vi;
			outForce *= dragPreCoeff * rij * outForce.norm();
		}

		template<bool applyDragOnFluid>
		static Storm::Vector3 computeSumDragForce(const Storm::IterationParameter &iterationParameter, const float uniformDragCoeff, const Storm::FluidParticleSystem &fluidParticleSystem, const Storm::Vector3 &vi, const float currentPDensity, const std::size_t currentPIndex)
		{
			Storm::Vector3 totalDragForce = Storm::Vector3::Zero();
			Storm::Vector3 currentDragTmpComponent = Storm::Vector3::Zero();

			const float dragPreCoeff = uniformDragCoeff * currentPDensity;

			auto dragForceComputeLambda = [&]<Storm::FluidParticleSystemUtils::NeighborType neighborType>(const Storm::NeighborParticleInfo &neighbor)
			{
				computeDragForce(vi, neighbor._containingParticleSystem->getVelocity()[neighbor._particleIndex], dragPreCoeff, neighbor._Wij, currentDragTmpComponent);
				totalDragForce += currentDragTmpComponent;

				if constexpr (neighborType == Storm::FluidParticleSystemUtils::NeighborType::DynamicRb)
				{
					// Mirror the force on the boundary solid following the 3rd newton law
					Storm::RigidBodyParticleSystem* neighborPSystemAsBoundary = static_cast<Storm::RigidBodyParticleSystem*>(neighbor._containingParticleSystem);
					Storm::Vector3 &boundaryNeighborTmpDragForce = neighborPSystemAsBoundary->getTemporaryDragForces()[neighbor._particleIndex];

					std::lock_guard<std::mutex> lock{ neighbor._containingParticleSystem->_mutex };
					boundaryNeighborTmpDragForce -= currentDragTmpComponent;
				}
			};

			if constexpr (applyDragOnFluid)
			{
				Storm::FluidParticleSystemUtils::forEachNeighbor(fluidParticleSystem, currentPIndex, dragForceComputeLambda);
			}
			else
			{
				Storm::FluidParticleSystemUtils::forEachRigidbodyNeighbor(fluidParticleSystem, currentPIndex, dragForceComputeLambda);
			}

			return totalDragForce;
		}
	};
}
