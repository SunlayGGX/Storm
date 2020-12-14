#pragma once


#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	enum class ThreadEnumeration;

	class IScriptManager : public Storm::ISingletonHeldInterface<IScriptManager>
	{
	public:
		virtual ~IScriptManager() = default;

	public:
		virtual void execute(const Storm::ThreadEnumeration threadEnum, std::string script) = 0;
		virtual void execute(std::string script) = 0;
	};
}
