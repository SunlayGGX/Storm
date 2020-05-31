#pragma once


#include "Singleton.h"
#include "IAssetLoaderManager.h"


namespace Storm
{
	class AssetLoaderManager :
		private Storm::Singleton<AssetLoaderManager>,
		public Storm::IAssetLoaderManager
	{
		STORM_DECLARE_SINGLETON(AssetLoaderManager);

	private:
		void initialize_Implementation();
		void cleanUp_Implementation();
	};
}
