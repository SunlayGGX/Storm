#pragma once

#include "IRigidBody.h"


namespace Storm
{
	struct RigidBodySceneData;
	class AssetCacheData;

	class RigidBody : public Storm::IRigidBody
	{
	public:
		struct ReplayMode{};

	public:
		RigidBody(const Storm::RigidBodySceneData &rbSceneData);
		RigidBody(const Storm::RigidBodySceneData &rbSceneData, ReplayMode);

	public:
		const std::string& getRigidBodyName() const final override;
		unsigned int getRigidBodyID() const final override;
		void getRigidBodyTransform(Storm::Vector3 &outTrans, Storm::Vector3 &outRot) const final override;
		void getRigidBodyTransform(Storm::Vector3 &outTrans, Storm::Quaternion &outRot) const final override;
		std::vector<Storm::Vector3> getRigidBodyParticlesWorldPositions() const final override;
		std::vector<Storm::Vector3> getRigidBodyObjectSpaceVertexes() const final override;
		std::vector<Storm::Vector3> getRigidBodyObjectSpaceNormals() const final override;

	public:
		static std::filesystem::path retrieveParticleDataCacheFolder();

	private:
		std::shared_ptr<Storm::AssetCacheData> baseLoadAssimp(const Storm::RigidBodySceneData &rbSceneData);

		void load(const Storm::RigidBodySceneData &rbSceneData);
		void loadForReplay(const Storm::RigidBodySceneData &rbSceneData);

	private:
		std::string _meshPath;
		unsigned int _rbId;
	};
}
