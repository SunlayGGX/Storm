
#include "Singleton.h"
#include "ISpacePartitionerManager.h"


namespace Storm
{
	class VoxelGrid;

	class SpacePartitionerManager final :
		private Storm::Singleton<SpacePartitionerManager>,
		public Storm::ISpacePartitionerManager
	{
		STORM_DECLARE_SINGLETON(SpacePartitionerManager);

	private:
		using SpacePartitionStructure = std::unique_ptr<Storm::VoxelGrid>;

	private:
		void initialize_Implementation(float partitionLength);
		void cleanUp_Implementation();

	public:
		void partitionSpace() final override;
		void clearSpaceReorderingNoStatic() final override;
		void computeSpaceReordering(const std::vector<Storm::Vector3> &particlePositions, Storm::PartitionSelection modality, const unsigned int systemId) final override;
		void clearSpaceReorderingForPartition(Storm::PartitionSelection modality) final override;
		void getAllBundles(const std::vector<Storm::NeighborParticleReferral>* &outContainingBundlePtr, const std::vector<Storm::NeighborParticleReferral>*(&outNeighborBundle)[Storm::k_neighborLinkedBunkCount], const Storm::Vector3 &particlePosition, Storm::PartitionSelection modality) const final override;
		void getContainingBundle(const std::vector<Storm::NeighborParticleReferral>* &outContainingBundlePtr, const Storm::Vector3 &particlePosition, Storm::PartitionSelection modality) const final override;

		float getPartitionLength() const final override;
		void setPartitionLength(float length) final override;

		bool isOutsideSpaceDomain(const Storm::Vector3 &position) const final override;

		bool isInfiniteDomainMode() const noexcept final override;

	public:
		std::shared_ptr<Storm::IDistanceSpacePartitionProxy> makeDistancePartitionProxy(const Storm::Vector3 &upCorner, const Storm::Vector3 &downCorner, const float partitionLength) final override;

	public:
		const std::unique_ptr<Storm::VoxelGrid>& getSpacePartition(Storm::PartitionSelection modality) const;

	private:
		Storm::Vector3 _upSpaceCorner;
		Storm::Vector3 _downSpaceCorner;

		float _partitionLength;
		Storm::Vector3 _gridShiftOffset;

		SpacePartitionStructure _fluidSpacePartition;
		SpacePartitionStructure _dynamicRigidBodySpacePartition;
		SpacePartitionStructure _staticRigidBodySpacePartition;

		bool _infiniteDomain;
	};
}
