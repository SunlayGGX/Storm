#pragma once


namespace Storm
{
	enum class GeometryType;

	class UniformSampler
	{
	public:
		// "dimensions" should be :
		// - the radius (a float) if geometry is a sphere
		// - Storm::Vector3 (dimension x, y, z) if geometry is a cube
		static std::vector<Storm::Vector3> process(const Storm::GeometryType geometry, const float separationDistance, const void*const dimensions);
	};
}
