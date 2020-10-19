#pragma once

#include "ParticleSystemContainer.h"


namespace Storm
{
	enum class SimulationMode;
	class ParticleSystem;

	class __declspec(novtable) ISPHBaseSolver
	{
	public:
		virtual ~ISPHBaseSolver() = default;

	public:
		virtual void execute(const Storm::ParticleSystemContainer &particleSystems, const float kernelLength, const float k_deltaTime) = 0;
	};

	std::unique_ptr<Storm::ISPHBaseSolver> instantiateSPHSolver(const Storm::SimulationMode simulationMode, const float kernelLength);
}
