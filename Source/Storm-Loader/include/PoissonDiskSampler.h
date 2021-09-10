#pragma once

#include "SamplingResult.h"


namespace Storm
{
	// https://medium.com/@hemalatha.psna/implementation-of-poisson-disc-sampling-in-javascript-17665e406ce1
	class PoissonDiskSampler
	{
	public:
		static Storm::SamplingResult process(const int kTryConst, const float diskRadius, const std::vector<Storm::Vector3> &vertices);
		static Storm::SamplingResult process_v2(const int kTryConst, const float diskRadius, const std::vector<Storm::Vector3> &vertices, const Storm::Vector3 &upCorner, const Storm::Vector3 &downCorner);
	};
}
