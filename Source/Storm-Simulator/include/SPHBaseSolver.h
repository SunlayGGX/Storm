#pragma once


namespace Storm
{
	enum class SimulationMode;
	class ParticleSystem;

	class __declspec(novtable) ISPHBaseSolver
	{
	public:
		virtual ~ISPHBaseSolver() = default;

	public:
		virtual void execute(const std::map<unsigned int, std::unique_ptr<Storm::ParticleSystem>> &particleSystems, const float kernelLength, const float k_deltaTime) = 0;
	};

	std::unique_ptr<Storm::ISPHBaseSolver> instantiateSPHSolver(const Storm::SimulationMode simulationMode);
}
