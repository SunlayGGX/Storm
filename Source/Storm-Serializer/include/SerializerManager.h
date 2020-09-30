#pragma once

#include "Singleton.h"
#include "ISerializerManager.h"


namespace Storm
{
	class SerializerManager :
		private Storm::Singleton<Storm::SerializerManager>,
		public Storm::ISerializerManager
	{
		STORM_DECLARE_SINGLETON(SerializerManager);

	private:
		void initialize_Implementation();
		void cleanUp_Implementation();

	private:
		void run();
		void execute();

	private:
		std::thread _serializeThread;
	};
}
