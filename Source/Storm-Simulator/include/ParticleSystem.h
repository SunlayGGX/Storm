#pragma once


namespace Storm
{
	class ParticleSystem
	{
	public:
		ParticleSystem(std::vector<Storm::Vector3> &&worldPositions);
		virtual ~ParticleSystem() = default;

	public:
		const std::vector<Storm::Vector3>& getPositions() const noexcept;
		const std::vector<Storm::Vector3>& getVelocity() const noexcept;
		const std::vector<Storm::Vector3>& getAccelerations() const noexcept;

		virtual bool isFluids() const noexcept = 0;

	protected:
		std::vector<float> _masses;
		std::vector<Storm::Vector3> _positions;
		std::vector<Storm::Vector3> _velocity;
		std::vector<Storm::Vector3> _accelerations;
	};
}
