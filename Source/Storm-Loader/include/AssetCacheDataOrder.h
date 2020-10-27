#pragma once


struct aiScene;

namespace Storm
{
	struct RigidBodySceneData;

	struct AssetCacheDataOrder
	{
	public:
		AssetCacheDataOrder(const Storm::RigidBodySceneData &rbConfig, const aiScene*const assimpScene, const float layerDistance);

	public:
		const Storm::RigidBodySceneData &_rbConfig;
		const aiScene* _assimpScene;
		bool _considerFinalInEquivalence;
		float _layerDistance;
	};
}
