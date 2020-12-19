#pragma once


#include "Singleton.h"
#include "IScriptManager.h"


namespace Storm
{
	class ScriptFile;
	class LuaScriptWrapper;

	class ScriptManager :
		private Storm::Singleton<ScriptManager>,
		public Storm::IScriptManager
	{
		STORM_DECLARE_SINGLETON(ScriptManager);

	private:
		void initialize_Implementation();
		void cleanUp_Implementation();

	public:
		void executeScript(const std::string &script); // Shouldn't be exposed to other modules!

	public:
		void execute(std::string script) final override;

	public:
		Storm::IScriptManager::UsedScriptWrapper& getScriptWrapper();

	private:
		void rereadWatchedScriptFile();
		void refreshWatchedScriptFile();

	private:
		std::unique_ptr<Storm::IScriptManager::UsedScriptWrapper> _scriptWrapper;

		std::chrono::high_resolution_clock::time_point _nextWatchedScriptFileUpdate;
		std::chrono::milliseconds _refreshTimeDuration;
		std::unique_ptr<Storm::ScriptFile> _watchedScriptFile;

		std::thread _scriptThread;
	};
}
