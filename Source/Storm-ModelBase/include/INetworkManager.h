#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	class INetworkManager : public Storm::ISingletonHeldInterface<Storm::INetworkManager>
	{
	public:
		virtual ~INetworkManager() = default;

	public:
	};
}
