#pragma once

#include "Facets.h"


namespace Storm
{
	class SingletonHeldInterfaceBase {};

	template<class ChildType>
	class ISingletonHeldInterface : public Storm::Facet<ISingletonHeldInterface<ChildType>, SingletonHeldInterfaceBase> 
	{
	public:
		virtual ~ISingletonHeldInterface() = default;
	};
}
