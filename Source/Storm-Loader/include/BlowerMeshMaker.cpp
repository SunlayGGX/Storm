#include "BlowerMeshMaker.h"

#include "AssetLoaderManager.h"

#include "BlowerData.h"
#include "BlowerType.h"

#include "CorrectSettingChecker.h"


#define STORM_ENSURE_MESH_MAKER_USED_ON_RIGHT_SETTING(BlowerDataVariable, ...) 						\
if (!CorrectSettingChecker<Storm::BlowerType>::check<__VA_ARGS__>(BlowerDataVariable._blowerType))	\
	Storm::throwException<Storm::StormException>(__FUNCTION__ " is intended to be used for " #__VA_ARGS__)	\


void Storm::BlowerCubeMeshMaker::generate(const Storm::BlowerData &blowerData, std::vector<Storm::Vector3> &outVertexes, std::vector<uint32_t> &outIndexes)
{
	STORM_ENSURE_MESH_MAKER_USED_ON_RIGHT_SETTING(blowerData, Storm::BlowerType::Cube);
	Storm::AssetLoaderManager::instance().generateSimpleCube(blowerData._blowerPosition, blowerData._blowerDimension, outVertexes, outIndexes);
}

void Storm::BlowerSphereMeshMaker::generate(const Storm::BlowerData &blowerData, std::vector<Storm::Vector3> &outVertexes, std::vector<uint32_t> &outIndexes)
{
	STORM_ENSURE_MESH_MAKER_USED_ON_RIGHT_SETTING(blowerData,
		Storm::BlowerType::Sphere, Storm::BlowerType::RepulsionSphere, Storm::BlowerType::ExplosionSphere, Storm::BlowerType::PulseExplosionSphere
	);

	Storm::AssetLoaderManager::instance().generateSimpleSphere(blowerData._blowerPosition, blowerData._radius, outVertexes, outIndexes);
}

void Storm::BlowerCylinderMeshMaker::generate(const Storm::BlowerData &blowerData, std::vector<Storm::Vector3> &outVertexes, std::vector<uint32_t> &outIndexes)
{
	STORM_ENSURE_MESH_MAKER_USED_ON_RIGHT_SETTING(blowerData, Storm::BlowerType::Cylinder);
	Storm::AssetLoaderManager::instance().generateSimpleCylinder(blowerData._blowerPosition, blowerData._radius, blowerData._height, outVertexes, outIndexes);
}

void Storm::BlowerConeMeshMaker::generate(const Storm::BlowerData &blowerData, std::vector<Storm::Vector3> &outVertexes, std::vector<uint32_t> &outIndexes)
{
	STORM_ENSURE_MESH_MAKER_USED_ON_RIGHT_SETTING(blowerData, Storm::BlowerType::Cone);
	Storm::AssetLoaderManager::instance().generateSimpleCone(blowerData._blowerPosition, blowerData._upRadius, blowerData._downRadius, blowerData._height, outVertexes, outIndexes);
}
