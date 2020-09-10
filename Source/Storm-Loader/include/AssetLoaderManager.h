#pragma once


#include "Singleton.h"
#include "IAssetLoaderManager.h"


namespace Storm
{
	class IRigidBody;
	class AssetCacheData;
	struct AssetCacheDataOrder;

	class AssetLoaderManager :
		private Storm::Singleton<AssetLoaderManager>,
		public Storm::IAssetLoaderManager
	{
		STORM_DECLARE_SINGLETON(AssetLoaderManager);

	private:
		void initialize_Implementation();
		void cleanUp_Implementation();

	public:
		const std::vector<std::shared_ptr<Storm::IRigidBody>>& getRigidBodyArray() const final override;

	public:
		void generateSimpleCube(const Storm::Vector3 &position, const Storm::Vector3 &dimension, std::vector<Storm::Vector3> &inOutVertexes, std::vector<uint32_t> &inOutIndexes) const final override;
		void generateSimpleSphere(const Storm::Vector3 &position, const float radius, std::vector<Storm::Vector3> &inOutVertexes, std::vector<uint32_t> &inOutIndexes) const final override;
		void generateSimpleCylinder(const Storm::Vector3 &position, const float radius, const float height, std::vector<Storm::Vector3> &inOutVertexes, std::vector<uint32_t> &inOutIndexes) const final override;
		void generateSimpleCone(const Storm::Vector3 &position, const float upRadius, const float downRadius, const float height, std::vector<Storm::Vector3> &inOutVertexes, std::vector<uint32_t> &inOutIndexes) const final override;

	public:
		// Retrieve the cached data asset. If the aiScene inside order is nullptr, then we would 
		std::shared_ptr<Storm::AssetCacheData> retrieveAssetData(const Storm::AssetCacheDataOrder &order);

	private:
		std::vector<std::shared_ptr<Storm::IRigidBody>> _rigidBodies;
		std::map<std::string, std::vector<std::shared_ptr<Storm::AssetCacheData>>> _cachedAssetData;
	};
}
