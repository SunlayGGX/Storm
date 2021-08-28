#pragma once


namespace Storm
{
	enum class GeometryType;

	class UniformSampler
	{
	public:
		// "samplerData" should be :
		// - the radius (a float) if geometry is a sphere
		// - Storm::Vector3 (dimension x, y, z) if geometry is a cube
		// - a std::pair<float, std::size_t> with first is radius and second is the particle count if geometry is a SphereTmp
		template<bool internalLayer>
		static std::vector<Storm::Vector3> process(const Storm::GeometryType geometry, const float separationDistance, const int layerCount, const void*const samplerData);
	};
}
