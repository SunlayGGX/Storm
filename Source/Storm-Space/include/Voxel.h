#pragma once


namespace Storm
{
	struct NeighborParticleReferral;

	class Voxel
	{
	public:
		Voxel();
		Voxel(const Storm::Voxel &other);
		Voxel(Storm::Voxel &&other);
		~Voxel();

	public:
		void clear();
		void addParticle(const std::size_t index, const unsigned int systemId);

		__forceinline const std::vector<Storm::NeighborParticleReferral>& getData() const noexcept { return _particleReferralsData; }

	private:
		std::vector<Storm::NeighborParticleReferral> _particleReferralsData;
	};
}
