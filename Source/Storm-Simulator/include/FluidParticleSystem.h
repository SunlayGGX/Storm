#pragma once


namespace Storm
{
	class FluidParticleSystem
	{
	public:
		FluidParticleSystem(std::vector<Storm::Vector3> &&worldPositions);

	public:
		const std::vector<Storm::Vector3>& getPositions() const;

	private:
		std::vector<Storm::Vector3> _positions;
	};
}
