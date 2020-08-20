#pragma once

#include "CRTPHierarchy.h"


namespace Storm
{
	enum class BlowerType;

	class IBlower :
		public Storm::FullHierarchisable<Storm::IBlower, std::size_t>
	{
	public:
		virtual ~IBlower() = default;

	public:
		virtual Storm::BlowerType getType() const = 0;
		virtual void advanceTime(float deltaTime) = 0;
		virtual void applyForce(const Storm::Vector3 &inParticlePosition, Storm::Vector3 &inOutParticleForce) const = 0;

		virtual bool operator==(const std::size_t id) const = 0;
		virtual bool operator<(const std::size_t id) const = 0;
	};
}
