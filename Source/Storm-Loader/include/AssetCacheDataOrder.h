#pragma once


struct aiScene;

namespace Storm
{
	struct SceneRigidBodyConfig;

	struct AssetCacheDataOrder
	{
	public:
		AssetCacheDataOrder(const Storm::SceneRigidBodyConfig &rbConfig, const aiScene*const assimpScene, const float layerDistance);

	public:
		const Storm::SceneRigidBodyConfig &_rbConfig;
		const aiScene* _assimpScene;
		bool _considerFinalInEquivalence;
		float _layerDistance;
	};
}
