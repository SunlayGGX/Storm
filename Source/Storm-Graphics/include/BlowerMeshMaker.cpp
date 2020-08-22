#include "BlowerMeshMaker.h"

#include "SingletonHolder.h"
#include "IAssetLoaderManager.h"

#include "BlowerData.h"
#include "BlowerType.h"

#include "ThrowException.h"


#define STORM_ENSURE_MESH_MAKER_USED_ON_RIGHT_SETTING(BlowerDataVariable, Setting) 				\
if (BlowerDataVariable._blowerType != Storm::Setting)											\
	Storm::throwException<std::exception>(__FUNCTION__ " is intended to be used for " #Setting)	\


void Storm::BlowerCubeMeshMaker::generate(const Storm::BlowerData &blowerData, std::vector<Storm::Vector3> &outVertexes, std::vector<uint32_t> &outIndexes)
{
	STORM_ENSURE_MESH_MAKER_USED_ON_RIGHT_SETTING(blowerData, BlowerType::Cube);

	const Storm::IAssetLoaderManager &assetLoaderMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IAssetLoaderManager>();
	assetLoaderMgr.generateSimpleCube(blowerData._blowerPosition, blowerData._blowerDimension, outVertexes, outIndexes);
}

void Storm::BlowerSphereMeshMaker::generate(const Storm::BlowerData &blowerData, std::vector<Storm::Vector3> &outVertexes, std::vector<uint32_t> &outIndexes)
{
	STORM_ENSURE_MESH_MAKER_USED_ON_RIGHT_SETTING(blowerData, BlowerType::Sphere);

	const Storm::IAssetLoaderManager &assetLoaderMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IAssetLoaderManager>();
	assetLoaderMgr.generateSimpleSphere(blowerData._blowerPosition, blowerData._radius, outVertexes, outIndexes);
}
