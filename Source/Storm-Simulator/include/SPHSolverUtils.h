#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	class SPHSolverUtils : private Storm::NonInstanciable
	{
	public:
		template<class DataContainerType>
		static void removeRawEndData(const unsigned int pSystemId, std::size_t toRemoveCount, DataContainerType &dataMap)
		{
			if (auto found = dataMap.find(pSystemId); found != std::end(dataMap))
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
		static Storm::Vector3 computeSumDragForce(const Storm::IterationParameter &/*iterationParameter*/, const float uniformDragCoeff, const Storm::FluidParticleSystem &fluidParticleSystem, const Storm::Vector3 &vi, const Storm::ParticleNeighborhoodArray &currentPNeighborhood, const float currentPDensity)
		{
			Storm::Vector3 totalDragForce = Storm::Vector3::Zero();
			Storm::Vector3 currentDragTmpComponent = Storm::Vector3::Zero();

			const float dendity0 = fluidParticleSystem.getRestDensity();
			const float currentPDensityRatio = dendity0 * dendity0 / currentPDensity;
			const float fluidDragPreCoeff = uniformDragCoeff * currentPDensityRatio * fluidParticleSystem.getParticleVolume();

			// Even though reflected particle are real particle inside the domain, they are in fact dummies to represent the notion of outside of domain particle
			// And we consider out of domain particle to be still air (or at least, we don't care about their velocity).
			// In other words, we consider reflected particle, aka out of domain particle, to have no velocity.
			Storm::Vector3 k_velocityInCasePReflected = Storm::Vector3::Zero();

			for (const Storm::NeighborParticleInfo &neighbor : currentPNeighborhood)
			{
				if (neighbor._isFluidParticle)
				{
					if constexpr (applyDragOnFluid)
					{
						computeDragForce(vi, neighbor._notReflected ? neighbor._containingParticleSystem->getVelocity()[neighbor._particleIndex] : k_velocityInCasePReflected, fluidDragPreCoeff, neighbor._Wij, currentDragTmpComponent);
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

						const Storm::Vector3 &vj = neighbor._notReflected ? neighborPSystemAsBoundary->getVelocity()[neighbor._particleIndex] : k_velocityInCasePReflected;

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
