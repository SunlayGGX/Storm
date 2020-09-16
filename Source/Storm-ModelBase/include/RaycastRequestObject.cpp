#include "RaycastHitResult.h"
#include "RaycastQueryRequest.h"


Storm::RaycastHitResult::RaycastHitResult(std::size_t particleId, unsigned int systemId, float hitDistance) :
	_particleId{ particleId },
	_systemId{ systemId },
	_hitDistance{ hitDistance }
{

}

Storm::RaycastQueryRequest::RaycastQueryRequest(Storm::HitResponseCallback &&callback) :
	_hitResponseCallback{ std::move(callback) },
	_minDistance{ 0.f },
	_maxDistance{ std::numeric_limits<float>::max() },
	_considerOnlyVisible{ true },
	_wantOnlyFirstHit{ false }
{
	_particleSystemSelectionFlag.reserve(3);
}

Storm::RaycastQueryRequest& Storm::RaycastQueryRequest::addPartitionFlag(Storm::PartitionSelection flag)
{
	if (std::find(std::begin(_particleSystemSelectionFlag), std::end(_particleSystemSelectionFlag), flag) == std::end(_particleSystemSelectionFlag))
	{
		_particleSystemSelectionFlag.emplace_back(flag);
	}

	return *this;
}


Storm::RaycastQueryRequest& Storm::RaycastQueryRequest::setMinDistance(float value)
{
	_minDistance = value;
	return *this;
}

Storm::RaycastQueryRequest& Storm::RaycastQueryRequest::setMaxDistance(float value)
{
	_maxDistance = value;
	return *this;
}

Storm::RaycastQueryRequest& Storm::RaycastQueryRequest::considerOnlyVisible(bool value)
{
	_considerOnlyVisible = value;
	return *this;
}

Storm::RaycastQueryRequest& Storm::RaycastQueryRequest::firstHitOnly()
{
	_wantOnlyFirstHit = true;
	return *this;
}
