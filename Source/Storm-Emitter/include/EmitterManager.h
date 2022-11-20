#pragma once


#include "Singleton.h"
#include "IEmitterManager.h"


namespace Storm
{
	class EmitterManager final :
		private Storm::Singleton<Storm::EmitterManager>,
		public Storm::IEmitterManager
	{
		STORM_DECLARE_SINGLETON(EmitterManager);

	private:
		void initialize_Implementation();
		void cleanUp_Implementation();
	};
}
