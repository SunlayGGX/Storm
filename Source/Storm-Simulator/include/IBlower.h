#pragma once


namespace Storm
{
	enum class BlowerType;

	class IBlower
	{
	public:
		virtual ~IBlower() = default;

	public:
		virtual Storm::BlowerType getType() const = 0;
		virtual void advanceTime(float deltaTime) = 0;
		virtual void applyForce(const Storm::Vector3 &inParticlePosition, Storm::Vector3 &inOutParticleForce) const = 0;
	};
}
