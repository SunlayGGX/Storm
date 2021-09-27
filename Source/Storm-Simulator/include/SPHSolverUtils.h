#pragma once

#include "NonInstanciable.h"


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
			// Squared is when we're in a turbulent system, but since we're laminar (for incompressibility purpose), then no squared.
			outForce *= dragPreCoeff * rij /** outForce.norm()*/;
		}

		template<bool applyDragOnFluid>
		static Storm::Vector3 computeSumDragForce(const Storm::IterationParameter &iterationParameter, const float uniformDragCoeff, const Storm::FluidParticleSystem &fluidParticleSystem, const Storm::Vector3 &vi, const Storm::ParticleNeighborhoodArray &currentPNeighborhood, const float currentPDensity)
		{
			Storm::Vector3 totalDragForce = Storm::Vector3::Zero();
			Storm::Vector3 currentDragTmpComponent = Storm::Vector3::Zero();

			const float currentPDensityRatio = fluidParticleSystem.getRestDensity() / currentPDensity;
			const float fluidDragPreCoeff = uniformDragCoeff * currentPDensityRatio * fluidParticleSystem.getParticleVolume();

			for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
			{
				if (neighbor._isFluidParticle)
				{
					if constexpr (applyDragOnFluid)
					{
						computeDragForce(vi, neighbor._containingParticleSystem->getVelocity()[neighbor._particleIndex], fluidDragPreCoeff, neighbor._Wij, currentDragTmpComponent);
						totalDragForce += currentDragTmpComponent;
					}
				}
				else
				{
					Storm::RigidBodyParticleSystem*const neighborPSystemAsBoundary = static_cast<Storm::RigidBodyParticleSystem*>(neighbor._containingParticleSystem);
					float rbDragPreCoeff = neighborPSystemAsBoundary->getDragCoefficient();
					if (rbDragPreCoeff > 0.f)
					{
						rbDragPreCoeff *= currentPDensityRatio * neighborPSystemAsBoundary->getVolumes()[neighbor._particleIndex];

						const Storm::Vector3 &vj = neighborPSystemAsBoundary->getVelocity()[neighbor._particleIndex];

						computeDragForce(vi, vj, rbDragPreCoeff, neighbor._Wij, currentDragTmpComponent);
						totalDragForce += currentDragTmpComponent;

						// Mirror the force on the boundary solid following the 3rd newton law
						if (!neighborPSystemAsBoundary->isStatic())
						{
							Storm::Vector3 &boundaryNeighborTmpDragForce = neighborPSystemAsBoundary->getTemporaryDragForces()[neighbor._particleIndex];

							std::lock_guard<std::mutex> lock{ neighbor._containingParticleSystem->_mutex };
							boundaryNeighborTmpDragForce -= currentDragTmpComponent;
						}
					}
				}
			}

			return totalDragForce;
		}
	};
}
