#pragma once

#include "Singleton.h"
#include "ISerializerManager.h"
#include "SingletonDefaultImplementation.h"


namespace Storm
{
	class SerializerManager :
		private Storm::Singleton<Storm::SerializerManager, Storm::DefineDefaultInitAndCleanupImplementation>,
		public Storm::ISerializerManager
	{
		STORM_DECLARE_SINGLETON(SerializerManager);
	};
}
