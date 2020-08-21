#pragma once


#include "Singleton.h"
#include "IAssetLoaderManager.h"


namespace Storm
{
	class IRigidBody;

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

	private:
		std::vector<std::shared_ptr<Storm::IRigidBody>> _rigidBodies;
	};
}
