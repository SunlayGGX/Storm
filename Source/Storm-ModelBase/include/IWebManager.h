#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	class IWebManager : public Storm::ISingletonHeldInterface<IWebManager>
	{
	public:
		virtual ~IWebManager() = default;

	public:
		virtual std::size_t openURL(const std::string_view &url) = 0;
	};
}
