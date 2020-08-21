#include "MeshMaker.h"


#include "SingletonHolder.h"
#include "IAssetLoaderManager.h"


void Storm::CubeMeshMaker::generate(const Storm::Vector3 &pos, const Storm::Vector3 &scale, std::vector<Storm::Vector3> &outVertexes, std::vector<uint32_t> &outIndexes)
{
	const Storm::IAssetLoaderManager &assetLoaderMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IAssetLoaderManager>();
	assetLoaderMgr.generateSimpleCube(pos, scale, outVertexes, outIndexes);
}

void Storm::SphereMeshMaker::generate(const Storm::Vector3 &pos, const Storm::Vector3 &scale, std::vector<Storm::Vector3> &outVertexes, std::vector<uint32_t> &outIndexes)
{
	if (scale.x() != scale.y() && scale.x() != scale.z())
	{
		Storm::throwException<std::exception>("This should be a true spheres (scale should be uniform on all coordinate but was " + Storm::toStdString(scale) + ")!");
	}

	const Storm::IAssetLoaderManager &assetLoaderMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IAssetLoaderManager>();
	assetLoaderMgr.generateSimpleSphere(pos, scale.x(), outVertexes, outIndexes);
}
