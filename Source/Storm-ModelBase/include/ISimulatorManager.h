#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	using SimulationCallback = std::function<void()>;
	struct BlowerData;

	class ISimulatorManager : public Storm::ISingletonHeldInterface<ISimulatorManager>
	{
	public:
		virtual ~ISimulatorManager() = default;

	public:
		virtual void addFluidParticleSystem(unsigned int id, std::vector<Storm::Vector3> particlePositions) = 0;
		virtual void addRigidBodyParticleSystem(unsigned int id, std::vector<Storm::Vector3> particlePositions) = 0;

		// Warning : returns a copy to avoid data races so be careful when using it...
		virtual std::vector<Storm::Vector3> getParticleSystemPositions(unsigned int id) const = 0;

	public:
		virtual void loadBlower(const Storm::BlowerData &blowerData) = 0;

	public:
		virtual float getKernelLength() const = 0;

	public:
		virtual void printFluidParticleData() const = 0;
	};
}
