#pragma once


namespace Storm
{
	struct IterationParameter;
	struct SolverCreationParameter;

	class __declspec(novtable) ISPHBaseSolver
	{
	public:
		virtual ~ISPHBaseSolver() = default;

	public:
		virtual void execute(const Storm::IterationParameter &iterationParameter) = 0;
	};

	std::unique_ptr<Storm::ISPHBaseSolver> instantiateSPHSolver(const Storm::SolverCreationParameter &creationParameter);
}
