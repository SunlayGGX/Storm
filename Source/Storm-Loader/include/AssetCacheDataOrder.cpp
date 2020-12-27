#include "AssetCacheDataOrder.h"


Storm::AssetCacheDataOrder::AssetCacheDataOrder(const Storm::SceneRigidBodyConfig &rbConfig, const aiScene*const assimpScene, const float layerDistance) :
	_rbConfig{ rbConfig },
	_assimpScene{ assimpScene },
	_considerFinalInEquivalence{ false },
	_layerDistance{ layerDistance }
{}
