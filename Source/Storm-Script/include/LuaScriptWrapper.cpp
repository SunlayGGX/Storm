#include "LuaScriptWrapper.h"

#include "ThreadingSafety.h"


namespace
{
	static int luaPrintOverride(lua_State* luaState)
	{
		int argsCount = lua_gettop(luaState);

		if (argsCount > 0)
		{
			const std::string_view msgPrefix = "Lua output :\n";

			std::string toPrint;
			toPrint.reserve(msgPrefix.size() + argsCount * 32);

			toPrint += msgPrefix;

			for (int iter = 1; iter <= argsCount; ++iter)
			{
				if (lua_isstring(luaState, iter))
				{
					std::size_t strLen;
					const char* msg = lua_tolstring(luaState, iter, &strLen);

					toPrint.append(msg, strLen);
				}
				else if (lua_isboolean(luaState, iter))
				{
					toPrint += Storm::toStdString(lua_toboolean(luaState, iter) != 0);
				}
				else if (lua_isnumber(luaState, iter))
				{
					toPrint += Storm::toStdString(lua_tonumberx(luaState, iter, NULL));
				}
				else if (lua_isinteger(luaState, iter))
				{
					toPrint += Storm::toStdString(lua_tointegerx(luaState, iter, NULL));
				}
				else
				{
					toPrint += "Cannot print custom type (";
					toPrint += lua_typename(luaState, iter);
					toPrint += ")";
				}

				toPrint += "\n";
			}

			LOG_SCRIPT_LOGIC << toPrint;
		}

		return 0;
	}

	static const luaL_Reg g_redefinor[] =
	{
		{ "print", luaPrintOverride },
		{ nullptr, nullptr }
	};
}


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


	// Redirect lua output to Storm logging module.

	lua_State* currentState = _lua.lua_state();
	lua_getglobal(currentState, "_G");

#define STORM_LUA_VERSION_IS_LOWER_THAN(major, minor) LUA_VERSION_NUM < major##0##minor
#if defined(LUA_VERSION_NUM) && STORM_LUA_VERSION_IS_LOWER_THAN(5, 2)
	luaL_register(currentState, nullptr, g_redefinor);
#else
	luaL_setfuncs(currentState, g_redefinor, 0);
#endif
#undef STORM_LUA_VERSION_IS_LOWER_THAN

	lua_pop(currentState, 1);
}

bool Storm::LuaScriptWrapper::execute(const std::string &scriptContent, std::string &inOutErrorMsg)
{
	assert(Storm::isScriptThread() && "This method should only be executed inside scripting thread!");

	auto result = _lua.safe_script(scriptContent);
	if (result.valid())
	{
		return true;
	}

	const sol::error errorRes = result;
	inOutErrorMsg += errorRes.what();

	result.abandon();

	return false;
}
