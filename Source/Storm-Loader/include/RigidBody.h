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
		Storm::Vector3 getRigidBodyWorldPosition() const final override;
		Storm::Vector3 getRigidBodyWorldRotation() const final override;
		Storm::Vector3 getRigidBodyWorldScale() const final override;
		const std::vector<Storm::Vector3>& getRigidBodyParticlesObjectSpacePositions() const final override;
		std::vector<Storm::Vector3> getRigidBodyObjectSpaceVertexes() const final override;
		std::vector<Storm::Vector3> getRigidBodyObjectSpaceNormals() const final override;

	private:
		void load();

	private:
		std::string _meshPath;
		unsigned int _rbId;
		std::vector<Storm::Vector3> _objSpaceParticlePos;
	};
}
