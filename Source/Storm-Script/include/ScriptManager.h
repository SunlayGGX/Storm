#pragma once


#include "Singleton.h"
#include "IScriptManager.h"


namespace Storm
{
	class ScriptFile;

	class ScriptManager :
		private Storm::Singleton<ScriptManager>,
		public Storm::IScriptManager
	{
		STORM_DECLARE_SINGLETON(ScriptManager);

	private:
		void initialize_Implementation();
		void cleanUp_Implementation();

	private:
		void executeScript(const std::string &script);

	public:
		void execute(const Storm::ThreadEnumeration threadEnum, std::string script) final override;
		void execute(std::string script) final override;

	private:
		void rereadWatchedScriptFile();
		void refreshWatchedScriptFile();

	private:
		std::vector<Storm::ScriptFile> _watchedScriptFiles;

		std::thread _scriptThread;
	};
}
