#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	class IProfilerManager : public Storm::ISingletonHeldInterface<Storm::IProfilerManager>
	{
	public:
		virtual ~IProfilerManager() = default;
	};
}
