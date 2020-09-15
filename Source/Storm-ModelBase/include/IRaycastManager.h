#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	enum class PartitionSelection;
	struct NeighborParticleReferral;

	using HitResponseCallback = std::function<void(std::vector<Storm::NeighborParticleReferral> &&)>;

	class IRaycastManager : public Storm::ISingletonHeldInterface<Storm::IRaycastManager>
	{
	public:
		virtual ~IRaycastManager() = default;

	public:
		virtual void queryRayCast(const Storm::Vector3 &origin, const Storm::Vector3 &direction, std::vector<Storm::PartitionSelection> &&hitFlag, HitResponseCallback callback) const = 0;
		virtual void queryRayCast(const Storm::Vector2 &pixelScreenPos, std::vector<Storm::PartitionSelection> &&hitFlag, HitResponseCallback callback) const = 0;
	};
}
