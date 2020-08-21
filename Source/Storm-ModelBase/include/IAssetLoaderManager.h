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

	public:
		// Simple mesh generation.

		virtual void generateSimpleCube(const Storm::Vector3 &position, const Storm::Vector3 &dimension, std::vector<Storm::Vector3> &inOutVertexes, std::vector<uint32_t> &inOutIndexes) const = 0;
		virtual void generateSimpleSphere(const Storm::Vector3 &position, const float radius, std::vector<Storm::Vector3> &inOutVertexes, std::vector<uint32_t> &inOutIndexes) const = 0;
	};
}
