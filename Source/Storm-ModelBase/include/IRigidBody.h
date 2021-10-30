#pragma once


namespace Storm
{
	// This class is for accessing rigid body parameters using the other underlying rigid body objects.
	// The others are engine related so don't use them outside of the underlying implementation of this one.
	class IRigidBody
	{
	public:
		// This is also the path to the rigid body.
		virtual const std::string& getRigidBodyName() const noexcept = 0;

		virtual unsigned int getRigidBodyID() const noexcept = 0;

		virtual void getRigidBodyTransform(Storm::Vector3 &outTrans, Storm::Rotation &outRot) const = 0;
		virtual void getRigidBodyTransform(Storm::Vector3 &outTrans, Storm::Quaternion &outRot) const = 0;

		// Warning : returns a copy to avoid data races so be careful when using it...
		virtual std::vector<Storm::Vector3> getRigidBodyParticlesWorldPositions() const = 0;
		virtual std::vector<Storm::Vector3> getRigidBodyObjectSpaceVertexes() const = 0;
		virtual std::vector<Storm::Vector3> getRigidBodyObjectSpaceNormals() const = 0;

		virtual float getRigidBodyVolume() const = 0;

		virtual bool isWall() const = 0;
	};
}
