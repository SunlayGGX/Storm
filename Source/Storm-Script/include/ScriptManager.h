#pragma once


#include "Singleton.h"
#include "IScriptManager.h"


namespace Storm
{
	class ScriptManager :
		private Storm::Singleton<ScriptManager>,
		public Storm::IScriptManager
	{
		STORM_DECLARE_SINGLETON(ScriptManager);

	private:
		void initialize_Implementation();
		void cleanUp_Implementation();

	public:

	};
}
