#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	class ParticleSystem;

	class WCSPHSolver : private Storm::NonInstanciable
	{
	public:
		static void execute(const std::map<unsigned int, std::unique_ptr<Storm::ParticleSystem>> &particleSystems, const float kernelLength);
	};

	class PCISPHSolver : private Storm::NonInstanciable
	{
	public:
		static void execute(const std::map<unsigned int, std::unique_ptr<Storm::ParticleSystem>> &particleSystems, const float kernelLength);
	};
}
