#pragma once


namespace Storm
{
	class SPHSolverPrivateLogic
	{
		// Since it is private logic, this should be inherited privately. Therefore no pointer with this type, and no need for virtual destructor.
	protected:
		~SPHSolverPrivateLogic() = default;

	protected:
		bool shouldContinue() const;

	protected:
		static constexpr const float k_epsilon = 0.000001f;
	};
}
