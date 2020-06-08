#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	using SimulationCallback = std::function<void()>;

	class ISimulatorManager : public Storm::ISingletonHeldInterface<ISimulatorManager>
	{
	public:
		virtual ~ISimulatorManager() = default;

	public:
		virtual void addFluidParticleSystem(unsigned int id, std::vector<Storm::Vector3> particlePositions) = 0;
		virtual void addRigidBodyParticleSystem(unsigned int id, std::vector<Storm::Vector3> particlePositions) = 0;
	};
}
