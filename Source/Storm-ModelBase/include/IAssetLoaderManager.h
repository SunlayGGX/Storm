#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	class IRigidBody;

	class IAssetLoaderManager : public Storm::ISingletonHeldInterface<IAssetLoaderManager>
	{
	public:
		virtual ~IAssetLoaderManager() = default;

	public:
		virtual const std::vector<std::shared_ptr<Storm::IRigidBody>>& getRigidBodyArray() const = 0;
		virtual std::shared_ptr<Storm::IRigidBody> getRigidBody(const unsigned int rbId) const = 0;

	public:
		// Simple mesh generation.

		virtual void generateSimpleSmoothedCube(const Storm::Vector3 &position, const Storm::Vector3 &dimension, std::vector<Storm::Vector3> &inOutVertexes, std::vector<uint32_t> &inOutIndexes, std::vector<Storm::Vector3>*const inOutNormals = nullptr) const = 0;
		virtual void generateSimpleSphere(const Storm::Vector3 &position, const float radius, std::vector<Storm::Vector3> &inOutVertexes, std::vector<uint32_t> &inOutIndexes, std::vector<Storm::Vector3>*const inOutNormals = nullptr) const = 0;
		virtual void generateSimpleCylinder(const Storm::Vector3 &position, const float radius, const float height, std::vector<Storm::Vector3> &inOutVertexes, std::vector<uint32_t> &inOutIndexes, std::vector<Storm::Vector3>*const inOutNormals = nullptr) const = 0;
		virtual void generateSimpleCone(const Storm::Vector3 &position, const float upRadius, const float downRadius, const float height, std::vector<Storm::Vector3> &inOutVertexes, std::vector<uint32_t> &inOutIndexes, std::vector<Storm::Vector3>*const inOutNormals = nullptr) const = 0;
	};
}
