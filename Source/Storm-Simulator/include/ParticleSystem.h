#pragma once


namespace Storm
{
	class ParticleSystem
	{
	public:
		ParticleSystem(std::vector<Storm::Vector3> &&worldPositions);

	public:
		const std::vector<Storm::Vector3>& getPositions() const;

	private:
		std::vector<Storm::Vector3> _positions;
	};
}
