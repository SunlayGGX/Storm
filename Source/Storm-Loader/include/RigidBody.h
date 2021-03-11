#pragma once

#include "IRigidBody.h"


namespace Storm
{
	struct SceneRigidBodyConfig;
	class AssetCacheData;
	struct SystemSimulationStateObject;

	class RigidBody : public Storm::IRigidBody
	{
	public:
		struct ReplayMode {};

	public:
		RigidBody(const Storm::SceneRigidBodyConfig &rbSceneConfig);
		RigidBody(const Storm::SceneRigidBodyConfig &rbSceneConfig, Storm::SystemSimulationStateObject &&state);
		RigidBody(const Storm::SceneRigidBodyConfig &rbSceneConfig, ReplayMode);

	public:
		const std::string& getRigidBodyName() const final override;
		unsigned int getRigidBodyID() const final override;
		void getRigidBodyTransform(Storm::Vector3 &outTrans, Storm::Rotation &outRot) const final override;
		void getRigidBodyTransform(Storm::Vector3 &outTrans, Storm::Quaternion &outRot) const final override;
		std::vector<Storm::Vector3> getRigidBodyParticlesWorldPositions() const final override;
		std::vector<Storm::Vector3> getRigidBodyObjectSpaceVertexes() const final override;
		std::vector<Storm::Vector3> getRigidBodyObjectSpaceNormals() const final override;

		float getRigidBodyVolume() const final override;

	public:
		static std::filesystem::path retrieveParticleDataCacheFolder();

	private:
		std::shared_ptr<Storm::AssetCacheData> baseLoadAssimp(const Storm::SceneRigidBodyConfig &rbSceneConfig, const float layerDist);

		void load(const Storm::SceneRigidBodyConfig &rbSceneConfig);
		void loadForReplay(const Storm::SceneRigidBodyConfig &rbSceneConfig);
		void loadFromState(const Storm::SceneRigidBodyConfig &rbSceneConfig, Storm::SystemSimulationStateObject &&state);

	private:
		std::string _meshPath;
		unsigned int _rbId;

		float _rbVolume;
	};
}
