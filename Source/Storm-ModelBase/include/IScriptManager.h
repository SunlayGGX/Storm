#pragma once


#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	enum class ThreadEnumeration;
	class LuaScriptWrapper;

	class IScriptManager : public Storm::ISingletonHeldInterface<IScriptManager>
	{
	public:
		using UsedScriptWrapper = Storm::LuaScriptWrapper;

	public:
		virtual ~IScriptManager() = default;

	public:
		virtual void execute(std::string script) = 0;
	};
}
