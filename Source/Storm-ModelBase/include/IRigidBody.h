#pragma once


namespace Storm
{
	// This class is for accessing rigid body parameters using the other underlying rigid body objects.
	// The others are engine related so don't use them outside of the underlying implementation of this one.
	class IRigidBody
	{
	public:
		// This is also the path to the rigid body.
		virtual const std::string& getRigidBodyName() const = 0;

		virtual unsigned int getRigidBodyID() const = 0;

		virtual Storm::Vector3 getRigidBodyWorldPosition() const = 0;
		virtual Storm::Vector3 getRigidBodyWorldRotation() const = 0;
		virtual Storm::Vector3 getRigidBodyWorldScale() const = 0;

		virtual const std::vector<Storm::Vector3>& getRigidBodyParticlesObjectSpacePositions() const = 0;

		virtual std::vector<Storm::Vector3> getRigidBodyObjectSpaceVertexes() const = 0;
		virtual std::vector<Storm::Vector3> getRigidBodyObjectSpaceNormals() const = 0;
	};
}
