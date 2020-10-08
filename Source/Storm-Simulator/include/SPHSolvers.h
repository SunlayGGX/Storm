#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	class ParticleSystem;
	class ParticleSelector;

	class WCSPHSolver : private Storm::NonInstanciable
	{
	public:
		static void execute(const std::map<unsigned int, std::unique_ptr<Storm::ParticleSystem>> &particleSystems, const float kernelLength, bool shouldRegisterTemporaryForces);
	};

	class PCISPHSolver : private Storm::NonInstanciable
	{
	public:
		static void execute(const std::map<unsigned int, std::unique_ptr<Storm::ParticleSystem>> &particleSystems, const float kernelLength, bool shouldRegisterTemporaryForces);
	};
}
