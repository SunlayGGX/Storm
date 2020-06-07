#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	class IThreadManager : public Storm::ISingletonHeldInterface<IThreadManager>
	{
	public:
		virtual ~IThreadManager() = default;

	public:
		virtual void nameCurrentThread(const std::wstring &newName) = 0;
	};
}
