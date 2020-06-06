#pragma once

#include "IRigidBody.h"


namespace Storm
{
	struct RigidBodySceneData;

	class RigidBody : public Storm::IRigidBody
	{
	public:
		RigidBody(const Storm::RigidBodySceneData &rbSceneData);

	public:
		const std::string& getRigidBodyName() const final override;
		unsigned int getRigidBodyID() const final override;
		void getRigidBodyTransform(Storm::Vector3 &outTrans, Storm::Vector3 &outRot) const final override;
		const std::vector<Storm::Vector3>& getRigidBodyParticlesObjectSpacePositions() const final override;
		std::vector<Storm::Vector3> getRigidBodyObjectSpaceVertexes() const final override;
		std::vector<Storm::Vector3> getRigidBodyObjectSpaceNormals() const final override;

	public:
		static std::filesystem::path retrieveParticleDataCacheFolder();

	private:
		void sampleMesh(const std::vector<Storm::Vector3> &vertices);

	private:
		void load(const Storm::RigidBodySceneData &rbSceneData);

	private:
		std::string _meshPath;
		unsigned int _rbId;
		std::vector<Storm::Vector3> _objSpaceParticlePos;
	};
}
