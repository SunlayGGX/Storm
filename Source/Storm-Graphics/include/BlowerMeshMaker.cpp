#include "BlowerMeshMaker.h"

#include "SingletonHolder.h"
#include "IAssetLoaderManager.h"

#include "BlowerData.h"
#include "BlowerType.h"

#include "ThrowException.h"
#include "CorrectSettingChecker.h"


#define STORM_ENSURE_MESH_MAKER_USED_ON_RIGHT_SETTING(BlowerDataVariable, ...) 						\
if (!CorrectSettingChecker<Storm::BlowerType>::check<__VA_ARGS__>(BlowerDataVariable._blowerType))	\
	Storm::throwException<std::exception>(__FUNCTION__ " is intended to be used for " #__VA_ARGS__)	\


void Storm::BlowerCubeMeshMaker::generate(const Storm::BlowerData &blowerData, std::vector<Storm::Vector3> &outVertexes, std::vector<uint32_t> &outIndexes)
{
	STORM_ENSURE_MESH_MAKER_USED_ON_RIGHT_SETTING(blowerData, Storm::BlowerType::Cube);

	const Storm::IAssetLoaderManager &assetLoaderMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IAssetLoaderManager>();
	assetLoaderMgr.generateSimpleCube(blowerData._blowerPosition, blowerData._blowerDimension, outVertexes, outIndexes);
}

void Storm::BlowerSphereMeshMaker::generate(const Storm::BlowerData &blowerData, std::vector<Storm::Vector3> &outVertexes, std::vector<uint32_t> &outIndexes)
{
	STORM_ENSURE_MESH_MAKER_USED_ON_RIGHT_SETTING(blowerData,
		Storm::BlowerType::Sphere, Storm::BlowerType::RepulsionSphere, Storm::BlowerType::ExplosionSphere, Storm::BlowerType::PulseExplosionSphere
	);

	const Storm::IAssetLoaderManager &assetLoaderMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IAssetLoaderManager>();
	assetLoaderMgr.generateSimpleSphere(blowerData._blowerPosition, blowerData._radius, outVertexes, outIndexes);
}

void Storm::BlowerCylinderMeshMaker::generate(const Storm::BlowerData &blowerData, std::vector<Storm::Vector3> &outVertexes, std::vector<uint32_t> &outIndexes)
{
	STORM_ENSURE_MESH_MAKER_USED_ON_RIGHT_SETTING(blowerData, Storm::BlowerType::Cylinder);

	const Storm::IAssetLoaderManager &assetLoaderMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IAssetLoaderManager>();
	assetLoaderMgr.generateSimpleCylinder(blowerData._blowerPosition, blowerData._radius, blowerData._height, outVertexes, outIndexes);
}
