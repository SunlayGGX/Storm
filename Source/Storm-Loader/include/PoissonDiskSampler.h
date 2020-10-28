#pragma once


namespace Storm
{
	// https://medium.com/@hemalatha.psna/implementation-of-poisson-disc-sampling-in-javascript-17665e406ce1
	class PoissonDiskSampler
	{
	public:
		static std::vector<Storm::Vector3> process(const int kTryConst, const float diskRadius, const std::vector<Storm::Vector3> &vertices);
		static std::vector<Storm::Vector3> process_v2(const int kTryConst, const float diskRadius, const std::vector<Storm::Vector3> &vertices, const Storm::Vector3 &upCorner, const Storm::Vector3 &downCorner);
	};
}
