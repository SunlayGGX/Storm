#pragma once


namespace Storm
{
	class PoissonDiskSampler
	{
	public:
		std::vector<Storm::Vector3> operator()(const std::vector<Storm::Vector3> &vertices) const;
	};
}
