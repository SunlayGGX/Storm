#pragma once

#include "Inheritance.h"

#define SOL_EXCEPTIONS_SAFE_PROPAGATION 1
#	include <sol/sol.hpp>
#undef SOL_EXCEPTIONS_SAFE_PROPAGATION


namespace Storm
{
	class LuaScriptWrapper
	{
	private:
		template<class ... BaseTypes>
		struct TypeRegisterer
		{
			//STORM_COMPILE_ERROR("BaseTypes should be bundled under Storm::Inheritance class!");
		};

		template<class ... BaseTypes>
		struct TypeRegisterer<Storm::Inheritance<BaseTypes...>>
		{
		public:
			template<class Type, class ... Args>
			static void registerType(LuaScriptWrapper &wrapper, const std::string& name, Args &&... args)
			{
				wrapper.getUnderlying().new_usertype<Type>(name, std::forward<Args>(args)...);
			}
		};

		template<class BaseType, class ... BaseTypes>
		struct TypeRegisterer<Storm::Inheritance<BaseType, BaseTypes...>>
		{
		public:
			template<class Type, class ... Args>
			static void registerType(LuaScriptWrapper &wrapper, const std::string& name, Args &&... args)
			{
				TypeRegisterer<Storm::Inheritance<BaseTypes...>>::registerType(wrapper, name, std::forward<Args>(args)..., sol::base_classes, sol::bases<BaseType>());
			}
		};

		template<class BaseType>
		struct TypeRegisterer<Storm::Inheritance<BaseType>>
		{
		public:
			template<class Type, class ... Args>
			static void registerType(LuaScriptWrapper &wrapper, const std::string& name, Args &&... args)
			{
				TypeRegisterer<Storm::Inheritance<>>::registerType(wrapper, name, std::forward<Args>(args)..., sol::base_classes, sol::bases<BaseType>());
			}
		};

	public:
		LuaScriptWrapper();

	public:
		template<class Type, class InheritanceField = Storm::Inheritance<>, class ... Args>
		LuaScriptWrapper& registerType(const std::string& name, Args &&... args)
		{
			TypeRegisterer<InheritanceField>::registerType<Type>(*this, name, std::forward<Args>(args)...);
			return *this;
		}

		template<class Type, class ... Args>
		LuaScriptWrapper& registerEnum(const std::string& name, Args &&... args)
		{
			_lua.new_enum<Type>(name, std::forward<Args>(args)...);
			return *this;
		}

		template<class Type>
		LuaScriptWrapper& registerInstance(Type* instancePtr, const std::string &name)
		{
			_lua.set(name, instancePtr);
			return *this;
		}

	public:
		bool execute(const std::string &scriptContent, std::string &inOutErrorMsg);

	public:
		sol::state& getUnderlying()
		{
			return _lua;
		}

	private:
		sol::state _lua;
	};
}
