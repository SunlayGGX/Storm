#pragma once


namespace Storm
{
	class ParticleSystem
	{
	public:
		ParticleSystem(std::vector<Storm::Vector3> &&worldPositions);
		virtual ~ParticleSystem() = default;

	public:
		const std::vector<Storm::Vector3>& getPositions() const;

		virtual bool isFluids() const noexcept = 0;

	protected:
		std::vector<Storm::Vector3> _positions;
	};
}
