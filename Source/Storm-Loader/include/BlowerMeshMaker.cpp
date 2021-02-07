#include "BlowerMeshMaker.h"

#include "AssetLoaderManager.h"

#include "SceneBlowerConfig.h"
#include "BlowerType.h"

#include "CorrectSettingChecker.h"


#define STORM_ENSURE_MESH_MAKER_USED_ON_RIGHT_SETTING(BlowerConfigVariable, ...) 						\
if (!CorrectSettingChecker<Storm::BlowerType>::check<__VA_ARGS__>(BlowerConfigVariable._blowerType))	\
	Storm::throwException<Storm::Exception>(__FUNCTION__ " is intended to be used for " #__VA_ARGS__)	\


void Storm::BlowerCubeMeshMaker::generate(const Storm::SceneBlowerConfig &blowerConfig, std::vector<Storm::Vector3> &outVertexes, std::vector<uint32_t> &outIndexes)
{
	STORM_ENSURE_MESH_MAKER_USED_ON_RIGHT_SETTING(blowerConfig, Storm::BlowerType::Cube, Storm::BlowerType::CubeGradualDirectional);
	Storm::AssetLoaderManager::instance().generateSimpleSmoothedCube(blowerConfig._blowerPosition, blowerConfig._blowerDimension, outVertexes, outIndexes);
}

void Storm::BlowerSphereMeshMaker::generate(const Storm::SceneBlowerConfig &blowerConfig, std::vector<Storm::Vector3> &outVertexes, std::vector<uint32_t> &outIndexes)
{
	STORM_ENSURE_MESH_MAKER_USED_ON_RIGHT_SETTING(blowerConfig,
		Storm::BlowerType::Sphere, Storm::BlowerType::RepulsionSphere, Storm::BlowerType::ExplosionSphere, Storm::BlowerType::PulseExplosionSphere
	);

	Storm::AssetLoaderManager::instance().generateSimpleSphere(blowerConfig._blowerPosition, blowerConfig._radius, outVertexes, outIndexes);
}

void Storm::BlowerCylinderMeshMaker::generate(const Storm::SceneBlowerConfig &blowerConfig, std::vector<Storm::Vector3> &outVertexes, std::vector<uint32_t> &outIndexes)
{
	STORM_ENSURE_MESH_MAKER_USED_ON_RIGHT_SETTING(blowerConfig, Storm::BlowerType::Cylinder, Storm::BlowerType::CylinderGradualMidPlanar);
	Storm::AssetLoaderManager::instance().generateSimpleCylinder(blowerConfig._blowerPosition, blowerConfig._radius, blowerConfig._height, outVertexes, outIndexes);
}

void Storm::BlowerConeMeshMaker::generate(const Storm::SceneBlowerConfig &blowerConfig, std::vector<Storm::Vector3> &outVertexes, std::vector<uint32_t> &outIndexes)
{
	STORM_ENSURE_MESH_MAKER_USED_ON_RIGHT_SETTING(blowerConfig, Storm::BlowerType::Cone);
	Storm::AssetLoaderManager::instance().generateSimpleCone(blowerConfig._blowerPosition, blowerConfig._upRadius, blowerConfig._downRadius, blowerConfig._height, outVertexes, outIndexes);
}
