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

	private:
		std::vector<std::shared_ptr<Storm::IRigidBody>> _rigidBodies;
	};
}
