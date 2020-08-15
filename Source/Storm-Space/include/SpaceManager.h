
#include "Singleton.h"
#include "ISpaceManager.h"


namespace Storm
{
	class VoxelGrid;

	class SpaceManager :
		private Storm::Singleton<SpaceManager>,
		public Storm::ISpaceManager
	{
		STORM_DECLARE_SINGLETON(SpaceManager);

	private:
		using BundleMap = std::map<unsigned int, std::unique_ptr<Storm::VoxelGrid>>;

	private:
		void initialize_Implementation(float partitionLength);
		void cleanUp_Implementation();

	public:
		void partitionSpace(unsigned int systemId, bool isFluid) final override;
		void computeSpaceReordering(const std::vector<Storm::Vector3> &particlePositions, unsigned int systemId, bool isFluid) final override;
		void getAllBundles(const std::vector<std::size_t>* &outContainingBundlePtr, std::vector<const std::vector<std::size_t> *> &outNeighborBundle, const Storm::Vector3 &particlePosition, unsigned int systemId, bool isFluid) const final override;

		float getPartitionLength() const final override;
		void setPartitionLength(float length) final override;

	private:
		void partitionSpace_Internal(BundleMap &spacePartitionMap, unsigned int systemId);

		const std::unique_ptr<Storm::VoxelGrid>& getSpacePartition(unsigned int systemId, bool isFluid) const;

	private:
		Storm::Vector3 _upSpaceCorner;
		Storm::Vector3 _downSpaceCorner;

		float _partitionLength;

		BundleMap _rigidBodySpacePartition;
		BundleMap _fluidSpacePartition;
	};
}
