#include "RaycastQueryRequest.h"


Storm::RaycastQueryRequest::RaycastQueryRequest(Storm::HitResponseCallback &&callback) :
	_hitResponseCallback{ std::move(callback) }
{

}

Storm::RaycastQueryRequest::~RaycastQueryRequest() = default;

