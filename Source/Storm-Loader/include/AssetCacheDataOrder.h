#pragma once


struct aiScene;

namespace Storm
{
	struct RigidBodySceneData;

	struct AssetCacheDataOrder
	{
	public:
		AssetCacheDataOrder(const Storm::RigidBodySceneData &rbConfig, const aiScene*const assimpScene);

	public:
		const Storm::RigidBodySceneData &_rbConfig;
		const aiScene* _assimpScene;
		bool _considerFinalInEquivalence;
	};
}
