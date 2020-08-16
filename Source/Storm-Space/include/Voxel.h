#pragma once


namespace Storm
{
	struct NeighborParticleReferral;

	class Voxel
	{
	public:
		Voxel();
		Voxel(const Storm::Voxel &other);
		~Voxel();

	public:
		void clear();
		void addParticle(const std::size_t index, const unsigned int systemId);

		const std::vector<Storm::NeighborParticleReferral>& getData() const;

	private:
		std::vector<Storm::NeighborParticleReferral> _particleReferralsData;
	};
}
