#pragma once


namespace Storm
{
	// https://medium.com/@hemalatha.psna/implementation-of-poisson-disc-sampling-in-javascript-17665e406ce1
	class PoissonDiskSampler
	{
	public:
		PoissonDiskSampler(float diskRadius, int kTryConst);

	public:
		std::vector<Storm::Vector3> operator()(const std::vector<Storm::Vector3> &vertices) const;
	
	private:
		int _kTryConst;
		float _diskRadius;
	};
}
