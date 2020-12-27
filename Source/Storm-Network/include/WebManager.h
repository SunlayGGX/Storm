#pragma once

#include "Singleton.h"
#include "IWebManager.h"

#include "SingletonDefaultImplementation.h"


namespace Storm
{
	class WebManager final :
		private Storm::Singleton<Storm::WebManager, Storm::DefineDefaultInitAndCleanupImplementation>,
		public Storm::IWebManager
	{
		STORM_DECLARE_SINGLETON(WebManager);

	public:
		std::size_t openURL(const std::string_view &url) final override;
	};
}
