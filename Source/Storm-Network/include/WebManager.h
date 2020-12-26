#pragma once

#include "Singleton.h"
#include "IWebManager.h"


namespace Storm
{
	class WebManager final :
		private Storm::Singleton<Storm::WebManager>,
		public Storm::IWebManager
	{
		STORM_DECLARE_SINGLETON(WebManager);

	private:
		void initialize_Implementation();
		void cleanUp_Implementation();

	public:

	};
}
