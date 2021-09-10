#pragma once

#include "SpacePartitionConstants.h"


namespace Storm
{
	class __declspec(novtable) IDistanceSpacePartitionProxy
	{
	public:
		virtual ~IDistanceSpacePartitionProxy() = default;

	public:
		virtual bool addDataIfDistanceUnique(const Storm::Vector3 &data, const float distanceSquared) = 0;
		virtual bool addDataIfDistanceUnique(const Storm::Vector3 &data, const float distanceSquared, const std::vector<Storm::Vector3>* &containingBundlePtr, const std::vector<Storm::Vector3>* (&neighborBundlePtr)[Storm::k_neighborLinkedBunkCount]) = 0;
		virtual std::vector<Storm::Vector3> getCompleteData() const = 0;
	};
}
