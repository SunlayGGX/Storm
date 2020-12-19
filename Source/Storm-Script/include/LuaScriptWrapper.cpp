#include "LuaScriptWrapper.h"

#include "ThrowException.h"
#include "ThreadingSafety.h"


Storm::LuaScriptWrapper::LuaScriptWrapper()
{
	_lua.open_libraries(
		sol::lib::base,
#if defined(DEBUG) || defined(_DEBUG)
		sol::lib::debug,
#endif
		sol::lib::string,
		sol::lib::math,
		sol::lib::io
	);
}

bool Storm::LuaScriptWrapper::execute(const std::string &script, std::string &inOutErrorMsg /*= std::string{}*/)
{
	assert(Storm::isScriptThread() && "This method should only be executed inside scripting thread!");

	auto result = _lua.script(script);
	if (result.valid())
	{
		return true;
	}

	const sol::error errorRes = result;
	inOutErrorMsg += errorRes.what();

	result.abandon();

	return false;
}
