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
	};
}
