#pragma once


#include "Singleton.h"
#include "IAssetLoaderManager.h"


namespace Storm
{
	class IRigidBody;
	class AssetCacheData;
	struct AssetCacheDataOrder;
	struct SystemSimulationStateObject;

	class AssetLoaderManager final :
		private Storm::Singleton<AssetLoaderManager>,
		public Storm::IAssetLoaderManager
	{
		STORM_DECLARE_SINGLETON(AssetLoaderManager);

	private:
		void initialize_Implementation();
		void cleanUp_Implementation();

	private:
		void initializeForReplay();
		void initializeForSimulation();

	public:
		const std::vector<std::shared_ptr<Storm::IRigidBody>>& getRigidBodyArray() const final override;

	public:
		void generateSimpleSmoothedCube(const Storm::Vector3 &position, const Storm::Vector3 &dimension, std::vector<Storm::Vector3> &inOutVertexes, std::vector<uint32_t> &inOutIndexes, std::vector<Storm::Vector3>*const inOutNormals = nullptr) const final override;
		void generateSimpleSphere(const Storm::Vector3 &position, const float radius, std::vector<Storm::Vector3> &inOutVertexes, std::vector<uint32_t> &inOutIndexes, std::vector<Storm::Vector3>*const inOutNormals = nullptr) const final override;
		void generateSimpleCylinder(const Storm::Vector3 &position, const float radius, const float height, std::vector<Storm::Vector3> &inOutVertexes, std::vector<uint32_t> &inOutIndexes, std::vector<Storm::Vector3>*const inOutNormals = nullptr) const final override;
		void generateSimpleCone(const Storm::Vector3 &position, const float upRadius, const float downRadius, const float height, std::vector<Storm::Vector3> &inOutVertexes, std::vector<uint32_t> &inOutIndexes, std::vector<Storm::Vector3>*const inOutNormals = nullptr) const final override;

	public:
		// Retrieve the cached data asset. If the aiScene inside order is nullptr, then we would not create an asset data if we don't find one (and would return nullptr instead).
		std::shared_ptr<Storm::AssetCacheData> retrieveAssetData(const Storm::AssetCacheDataOrder &order);

	private:
		void clearCachedAssetData();

	private:
		void removeRbInsiderFluidParticle(std::vector<Storm::Vector3> &inOutFluidParticles, Storm::SystemSimulationStateObject* inOutSimulStateObjectPtr) const;

	public:
		std::mutex& getAddingMutex() const;
		std::mutex& getAssetMutex(const std::string &assetUID, bool &outMutexExistedBefore);

	private:
		std::vector<std::shared_ptr<Storm::IRigidBody>> _rigidBodies;
		std::map<std::string, std::vector<std::shared_ptr<Storm::AssetCacheData>>> _cachedAssetData;

		mutable std::mutex _addingMutex;
		mutable std::mutex _assetGeneralMutex;
		std::map<std::string, std::mutex> _assetSpecificMutexMap;
	};
}
