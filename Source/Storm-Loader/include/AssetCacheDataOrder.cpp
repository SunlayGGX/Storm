#include "AssetCacheDataOrder.h"


Storm::AssetCacheDataOrder::AssetCacheDataOrder(const Storm::RigidBodySceneData &rbConfig, const aiScene*const assimpScene) :
	_rbConfig{ rbConfig },
	_assimpScene{ assimpScene },
	_considerFinalInEquivalence{ false }
{}
