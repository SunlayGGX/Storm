#pragma once


#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	enum class ThreadEnumeration;

	class IBibliographyManager : public Storm::ISingletonHeldInterface<Storm::IBibliographyManager>
	{
	public:
		virtual ~IBibliographyManager() = default;

	public:
		virtual void generateBibTexLibrary() const = 0;
	};
}
